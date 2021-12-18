### OSFALL2021 TEAM10 PROJ4

## 0. How to build kernel
### 0.1 configuration change
arch/arm64/configs/tizen_bcmrpi_defconfig 파일에서 
`CONFIG_EXT2_FS is not set`을 지우고 `CONFIG_EXT2_FS=y, ` `# CONFIG_EXT@_FS_XATTR is not set`을 추가한다.

----수정 필요----
```
cd tizen-kernel/tizen-5.0-rpi3
git pull origin proj4
sudo sh compile.sh
sudo sh ./qemu.sh
```


## 1. High Level Implementation
### 1.1 Systemcall implementation
* include/linux/syscalls.h
    가장 아래에 다음을 추가한다. 
    ```
    asmlinkage long sys_set_gps_location(struct gps_location __user *loc);
    asmlinkage long sys_get_gps_location(const char __user *pathname, struct gps_location __user *loc);
    ```
* arch/arm/tools/syscalls.tbl
    가장 아래에 다음을 추가한다.
    ```
    398 common set_gps_location sys_set_gps_location
    399 common get_gps_location sys_get_gps_location
    ```
* arch/arm64/include/asm/unistd.h
    `#define __NR_compat_syscalls   400`
    로 수정한다.  
* arch/arm64/include/asm/unistd32.h
    아래 두 줄을 추가한다.  ::
    ```#define __NR_set_gps_location 398
    __SYSCALL(__NR_set_gps_location, sys_set_gps_location)
    #define __NR_get_gps_location 399
    __SYSCALL(__NR_get_gps_location, sys_get_gps_location)
    ```
이후 kernel/gps.c에  
```
SYSCALL_DEFINE1(set_gps_location, struct gps_location __user *, loc)
SYSCALL_DEFINE2(get_gps_location, const char __user *pathname, struct gps_location __user *loc)
```

### 1.2 include/linux/gps.h
gps.h 에는 struct gps_location 구조체를 정의하였고, 현재 시스템에 대한 location정보를 저장하는 systemloc을 정의해 주었다.
* gps_location
gps_location는 struct로 위도, 경도, accuracy에 대한 변수를 가지고 있다.
* struct gps_location systemloc
현재 시스템의 위도, 경도 accuracy에 대한 정보를 저장한다.

### 1.3 kernel/gps.c
#### 1.3.1 int valid_location(struct gps_location *loc)
argument로 gps_location struct를 받아서 해당 location정보가 valid한지 확인한 후 valid하면 1 아니면 0을 return한다.
lat_frac, lng_frac이 0과 999999사이에 있는 값인지 확인하고, lat_int가 -90부터 90사이의 값인지 확인하고 lng_int가 -180과 180사이의 값인지 확인하고 accuracy가 양수인지 확인한다.

#### 1.3.2 long set_gps_location(struct gps_location __user *loc)
loc값이 valid한지 확인하고 valid하다면 systemlocation의 위도, 경도, accuracy 값을 loc의 해당 값으로 바꾼다. systemlocation의 값을 수정할 때에는 해당 값이 다른 곳에서 참조되거나 수정되는 것을 막기 위해 spinlock을 잡고 수정한다.

#### 1.3.3 long get_gps_location(const char __user *pathname, struct gps_location __user *loc)
pathname이 valid한지 확인한 후 user에 있는 pathname data를 kernerl data에 복사해온다. pathname에 해당하는 파일의 inode값을 찾고 오류가 발생했는지 검사한다. 오류가 발생하지 않았다면 해당 파일이 gps_coordinate system을 사용하는지 확인하고 사용한다면 get_gps_location함수를 호출하여 inode에 저장되어 있는 location 정보를 읽은 후 systemloc의 location과 비교하여 파일에 저장된 위치가 accuracy범위 내에 있다면 해당 location struct의 값들을 loc 버퍼에 넣어준다.

### 1.4 fs/ext2/ext2.h
memory에 있는 inode data를 저장하는 ext2_inode_info 구조체에 __u32 자료형 변수 i_lat_integer, i_lat_fractional, i_lng_integer, i_lng_fractional, i_accuracy를 추가한다.


### 1.5 fs/ext2/file.c
const struct inode_opearations ext2_file_inode_operations 구조체의 set_gps_location과 get_gps_location 변수의 값을 각각 ext2_set_gps_location, ext2_get_gps_location으로 설정한다.


### 1.6 fs/ext2/inode.c
#### 1.6.1 ext2_set_gps_location(struct inode *inode)
argument로 받은 inode값을 현재의 systemlocation 값으로 설정해주는 함수이다. EXT2_I(inode)를 이용하여 ext2 파일 시스템 memory의 inode인 ext2_inode_info에 접근한 후 i_lat_integer, i_lat_fractional, i_lng_integer, i_lng_fractional, accuracy 값을 systemlocation의 위도, 경도, accuracy값으로 바꿔준다.

####1.6.2 ext2_get_gps_location(struct inode *inode, struct gps_location *loc)
위와 마찬가지로 ext2_inode_info 값을 얻은 후 i_lat_integer, i_lat_fractional, i_lng_integer, i_lng_fractional, accuracy 값을 gps_location 버퍼에 넣어준다.

### 1.7 include/linux/fs.h
inode_operations 구조체에 int(*set_gps_location)(struct inode *), int (*get_gps_location)(struct inode*, struct gps_location*) 두 개의 변수를 추가한다.

### 1.8 fs/ext2/namei.c
ext2 파일시스템에서 file을 만들 때 호출하는 함수인 ext2_create함수가 ext2_set_gps_location함수를 호출하여 파일을 만들 때 현재 systemlocation의 위도 경도 값을 해당 파일의 inode가 저장하도록 한다.


##2. Evaluation
###2.1 Test Files
####2.1.1 test/file_loc.c
파일의 이름 값을 argument로 받은 후 get_gps_location 시스템 콜을 호출하여 해당 파일의 inode에 저장되어 있는 위도 경도 값 얻은 후 해당 값을 콘솔에 출력한다. 해당 위도 경도 값에 해당하는 구글 맵 링크도 출력한다.
