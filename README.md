### OSFALL2021 TEAM10 PROJ4

## 0. How to build kernel


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
loc값이 valid한지 확인하고 valid하다면 systemloc의 위도, 경도, accuracy 값을 loc의 해당 값으로 바꾼다.

#### 1.3.3 long get_gps_location(const char __user *pathname, struct gps_location __user *loc)
pathname에 해당하는 파일을 찾고 해당 파일의 inode에 저장되어 있는 location 정보를 읽은 후 systemloc의 location과 비교하여 파일에 저장된 위치가 accuracy범위 내에 있다면 해당 location struct의 값들을 loc 버퍼에 넣어준다.

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

