### OSFALL2021 TEAM10 PROJ4

## 0. How to build kernel
### 0.1 configuration change
arch/arm64/configs/tizen_bcmrpi_defconfig 파일에서  
`CONFIG_EXT2_FS is not set`를 삭제한다.  
`CONFIG_EXT2_FS=y, `  
`# CONFIG_EXT2_FS_XATTR is not set` 두 줄을 추가하고  
`CONFIG_EXT4_USE_FOR_EXT2=y`를 `# CONFIG_EXT4_USE_FOR_EXT2 is not set`로 바꾼다.  
해당 파일을 tizen_bcmrpi_defconfig라고 저장한다.


### 0.2 compile
디렉토리 구조는 다음과 같다.
```bash
proj4
├─ mnt_dir
├─ tizen-5.0-rpi3
└─ tizen-image
```

다음과 같은 순서로 진행한다.
1. 다음을 실행하여 proj4 브랜치의 파일을 받는다.
```bash
git pull origin master
git branch proj4
git checkout proj4
git pull origin proj4
```

2. 이후 다음을 실행하여 컴파일을 하고 이미지 파일을 생성한다.
```bash
./build-rpi3-arm64.sh
sudo ./scripts/mkbootimg_rpi3.sh
```

3. config 파일을 tizen_bcmrpi3_defconfig_수정 으로 바꾸고 이름을 tizen_bcmrpi3_defconfig로 바꾼다.그리고 똑같은 코드를 이용하여 컴파일하고 boot.img, modules.img파일을 tizen-image디렉토리로 옮긴다.
4. 
```bash
./build-rpi3-arm64.sh
sudo ./scripts/mkbootimg_rpi3.sh
sudo mv boot.img modules.img ../tizen-image/
```

6. tizen-image에 압축파일을 해제하고, rootfs.img를 mnt_dir에 마운트한다.
```bash
cd ../tizen-image
tar xvzf tizen-unified_20181024.1_iot-headless-2parts-armv7l-rpi3.tar.gz
sudo mount rootfs.img ../mnt_dir
cd ../tizen-5.0-rpi3
```

7. test file을 compile하고 mnt_dir의 root디렉토리로 옮긴다.
```bash
cd test
make
sh move.sh
cd ..
```

8. e2fsprogs를 컴파일하고 proj.fs파일을 만들고 옮긴다. mnt_dir의 root 디렉토리로 옮긴다.
```bash
cd ./e2fsprogs
./configure
make
cd ../
```

sudo losetup -f를 이용하여 비어있는 loop device를 찾는다. 해당 디바이스가 loop14라고 가정하자.

```bash
dd if=/dev/zero of=proj4.fs bs=1M count=1
sudo losetup /dev/loop14 proj4.fs
sudo ./e2fsprogs/misc/mke2fs -I 256 -L os.proj4 /dev/loop14
sudo losetup -d /dev/loop14
sudo mv proj4.fs ../mnt_dir/root/
```

9. /etc/fstab를 루트 파일시스템이 write를 할 수 있도록 바꾼다.
다음과 같이 바꾼다.
```bash
# <file system> <mount point>   <type>  <options>           <dump> <pass>
/dev/root       /               ext4    defaults,noatime,rw 0      1
LABEL=system-data /opt          ext4    defaults,noatime    0      2
```

10. qemu를 실행한다.  
`sudo ./qemu.sh`

11. qemu shell 에서 proj4를 만들고 mount를 한다.
```bash
mkdir proj4;
mount -o loop -t ext2 /root/proj4.fs /root/proj4
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

#### 1.3.4 int LocationCompare(struct gps_location *locA, struct gps_location *locB)
두 gps 위치의 accuracy와 두 좌표간 실제 거리를 비교하여 접근 가능한지를 return한다.  
중간에 사용하는 함수인 get_dist는 두 좌표간 거리의 제곱을 return하므로 비교할때도 두 accuracy의 합의 제곱과 비교한다.  
distance가 accuracy의 합보다 더 작거나 같다면 accessible하므로 1을 return, 그렇지 않다면 0을 return한다.

### 1.4 fs/ext2/ext2.h
memory에 있는 inode data를 저장하는 ext2_inode_info 구조체에 __u32 자료형 변수 i_lat_integer, i_lat_fractional, i_lng_integer, i_lng_fractional, i_accuracy를 추가한다.


### 1.5 fs/ext2/file.c
const struct inode_opearations ext2_file_inode_operations 구조체의 set_gps_location과 get_gps_location 변수의 값을 각각 ext2_set_gps_location, ext2_get_gps_location으로 설정한다.


### 1.6 fs/ext2/inode.c
#### 1.6.1 ext2_set_gps_location(struct inode *inode)
argument로 받은 inode값을 현재의 systemlocation 값으로 설정해주는 함수이다. EXT2_I(inode)를 이용하여 ext2 파일 시스템 memory의 inode인 ext2_inode_info에 접근한 후 i_lat_integer, i_lat_fractional, i_lng_integer, i_lng_fractional, accuracy 값을 systemlocation의 위도, 경도, accuracy값으로 바꿔준다.

#### 1.6.2 ext2_get_gps_location(struct inode *inode, struct gps_location *loc)
위와 마찬가지로 ext2_inode_info 값을 얻은 후 i_lat_integer, i_lat_fractional, i_lng_integer, i_lng_fractional, accuracy 값을 gps_location 버퍼에 넣어준다.

#### 1.6.3 update gps_location when file is modified & convert struct between disk and memory
file이 create되거나 modify 될 때 inode 구조체의 ctime이나 mtime이 바뀌게 된다는 것을 생각하고 fs/ext2 디렉토리 내 파일들 중 ctime이나 mtime을 바꾸는 함수들을 모두 읽어보고, 이중에 필요한 함수들에 대해서 gps_location을 최신화 해주는 함수를 추가해주었다.  
ext2.h에 있는 두 구조체는 disk에 있을 때와 memory에 올라와 있을 때 두 경우 모두 gps_location에 대한 정보가 필요했는데, 이때 서로 다른 타입을 사용하므로 convert해주는 부분을 inode.c에서 찾게 되어 inode 구조체의 다른 변수들이 convert 될 때 이들도 동일한 방식으로 해주었다.

### 1.7 include/linux/fs.h
inode_operations 구조체에 int(*set_gps_location)(struct inode *), int (*get_gps_location)(struct inode*, struct gps_location*) 두 개의 변수를 추가한다.

### 1.8 fs/ext2/namei.c
ext2 파일시스템에서 file을 만들 때 호출하는 함수인 ext2_create함수가 ext2_set_gps_location함수를 호출하여 파일을 만들 때 현재 systemlocation의 위도 경도 값을 해당 파일의 inode가 저장하도록 한다.  
이 함수가 ext2_new_inode 함수를 call 하는 것을 보고 그 함수가 ctime을 current time으로 변경해주는 것을 보았으나 ialloc.c에 있는 이 함수는 다른 dir을 만들거나 link를 할 때도 자주 불리는 함수이므로 이 함수에 set gps를 하는 것은 불필요하다고 생각하여 ext2_create에만 넣어주었다.

### 1.9 fs/namei.c
int __inode_permission 함수에서 inode->i_op->get_gps_location 함수 포인터가 존재하는지 먼저 체크하고, 존재한다면 해당 fs를 사용한다는 의미이므로 LocationCompare을 이용하여 체크하고, 거리가 멀다면 EACCES error를 return하도록 하여 permission을 두었다. inode_permission과 관련된 다른 함수에 implement해도 괜찮겠다고 생각했으나 중간부분에 implement 하였다.

## 2. Fixed Point Operation
### 2.1 Formula to get distance between two points using latitude and longitude
아주 정확하다고 알려진 공식에는 acos, atan 등을 사용하는데 이것을 fixed point operation으로 나타내기 위해서 필요한 식들이 너무 많고 복잡하여 정밀도를 포기하고 최대한 간단하게 구성하기 위해 근사식들을 찾아보았다.  
지구를 완벽한 구 라고 가정하고, 위도는 다른 변수 없이 위도 간 거리는 항상 각도차이에 비례하므로 간단하게 구성하고, 경도 차이는 위도 식과 같으면서 (경도 차이에 비례한다는 경우가 같다) 위도의 cos값에 비례하므로 식을 간단히 구성할 수 있었다.  
위도, 경도는 보통 radian을 쓰지 않고 degree를 사용하므로 radian으로 보정해주는 것이 필요하였다.  
다만 여기서 식이 경도 차이는 두 점 중 어느 하나의 위도를 기준으로 경도차이를 계산하므로 사실 정확하지 않다.  
이를 어느정도 보정하기 위해 두 위도의 평균을 cos의 각도로 사용해주었다. 즉 경도차이가 심할수록 값의 정밀도가 크게 떨어질 것이다.  
```
지구 반지름 R = 6371000(m)
PI = 3.141592
위도 차이에 대한 거리 = R * (PI / 180) * |deg1 - deg2| = 111195 * |lat1 - lat2| (m)
경도 차이에 대한 거리 = cos ((lat1 + lat2)/2 * (PI / 180)) * 111195 * |lng1 - lng2| (m)
PI/180 = 0.017453 (degree to radian)
```

### 2.2 Struct for fixed point value
fixed point의 원활한 계산을 위해 새로운 구조체를 정의하였다.
fraction 부분의 최대 값이 999,999 인 만큼 multiplication 등의 계산을 할 때 int로만 할 경우 overflow가 날 수 있으므로 long long int를 통해 64bit으로 늘려 overflow를 최대한 방지해주었다.
```
typedef struct _fixed {
    long long int int_f;
    long long int frac_f;
} fixed;
```

### 2.3 Constants for calculating distance
float이나 double과 같은 소수점 연산 및 사용이 제한되어 있으므로   
필요하다고 판단된 constant들을 fixed로 만들어주었다. 먼저 지구를 완벽한 구로 가정하였을 때, radian으로 위도가 1 차이날 때 거리가 6371000 * PI / 180 = 111195(m)이다.
이는 경도를 계산할 때도 동일하게 적용된다.
또한 PI/180 = 0.017453 이므로 이도 fixed 구조체로 만들어준다.  
fractional part는 실제 값으로 사용할 때 값에 1,000,000을 나누어주어 사용하므로 보정한다는 의미로 CORRECTION (1000000LL)을 정의하여 사용하였다.

### 2.4 add, sub, mul, div
add, sub, mul은 그리 어렵지 않았다. fractional part는 항상 양수이고 999,999보다 작도록만 보장해주면 되었다.  
div는 많은 경우를 고려하면 너무 복잡해지므로, integer로 나누는 경우만 생각해주었다. div함수가 사용되는 경우가 cos밖에 없고 이때는 integer로만 나누므로 fixed point를 정밀도만큼 곱하여 큰 integer 값으로 바꿔주고 나누어 계산하였다. 이것도 매우 합리적인 것이 radius는 값이 -2pi에서 2pi 사이이므로 overflow를 걱정하지 않아도 되어 간단하게 구성하였다.

### 2.5 pow_f, factorial
둘 다 cos 계산에서만 사용되는데, 경우가 한정적이라는 것이 식을 간단하게 구성하는데 도움이 많이 되었다.
pow_f는 곱하는 수 들이 수가 작고, factorial도 작은 integer만 생각하면 되므로 overflow를 걱정하지 않아도 되었다.
pow_f는 mul을 이용하여 recursive로 간단하게 구성하였고, factorial도 recursive로 integer를 계산하였다.

### 2.6 cos_f, getdistance
#### 2.6.1 cos_f
cos은 생각보다 적은 iteration 만으로도 값이 잘 수렴하였다. 테일러 급수를 이용하였는데, 약 10번의 iteration하면서 값을 해보니 8~9번부터 값이 변하지 않아 8항까지 더하였다.
#### 2.6.2 get_dist
사실 위도 경도가 많이 차이나건, 적게 차이나건 모든 상황에 대해서 정확한 값을 구하기는 힘들다고 생각하였다. 구글맵이나 네이버 지도 같은 것을 사용할 때는 위도와 경도 차이가 아주 작은 경우가 많을 것이라고 예상이 되었으므로 위도 경도 차이가 큰 경우의 정밀도를 포기하였다. 이 경우에도 값이 overflow가 나지 않는 한 accessible 여부는 잘 판단해 줄 것이다. (gps의 정확도가 엄청나게 좋지 않아 accuracy가 엄청나게 큰 경우를 제외하고)  
sqrt 함수를 추가적으로 정의하는 것은 redundant하다고 판단하여 거리의 제곱을 return하도록 하였다. 마지막 compare 함수에서도 두 gps의 accuracy의 합의 제곱과 비교하면 될것이다. long long 변수가 생각보다 capacity가 매우 커서 제곱을 해도 상관 없다고 판단하였다.
구글 맵을 이용하여 직접 약 1km 내외의 좌표들을 이용하여 실제 실험해보니 값이 생각보다 굉장히 정확하였다. 다만 다양한 범위에 대해 실험해보지는 않았다.

## 3. Evaluation
### 3.1 Test Files
#### 3.1.1 test/file_loc.c
파일의 이름 값을 argument로 받은 후 get_gps_location 시스템 콜을 호출하여 해당 파일의 inode에 저장되어 있는 위도 경도 값 얻은 후 해당 값을 콘솔에 출력한다. 해당 위도 경도 값에 해당하는 구글 맵 링크도 출력한다.

## 4. Lesson Learned
1. 위도와 경도를 통해 두 점 간의 거리를 계산하는 것 자체는 사실 그렇게 복잡하지 않았지만, fixed point를 사용하면서 문제가 어렵게 되었으며 계산 정밀도에 대해 고민을 많이 하게 되었다. 실제로 테스트 해보니 삼각함수 자체는 굉장히 빠른속도로 수렴하고 (polynomial로 근사할 경우 나누는 항에 팩토리얼이 있어 매우 빠르게 수렴하는 것이라는 글을 보았다), acos, asin, atan 등의 연산은 다항함수로 표현하면 빠르게 수렴하지 않으므로 보통 다른 방식을 많이 사용한다는 것을 알게 되었다. 문제는 위도와 경도를 통한 거리계산에서 아주 정확한 식은 항상 acos이나 atan을 가지고 있었고 fixed point operation을 여러 계산을 지원하도록 아주 정밀하게 짠다면 물론 가장 정확한 결과를 얻을 수 있었겠지만 그것이 쉽지 않은 상황이었다. 그런 상황에서 정밀도가 떨어지는 acos, atan 등을 사용하게 되었을 때가 더 정확할지 아니면 연산 하나 하나가 정밀도가 조금 떨어지는 대신 간단한 연산들 만으로 대략적 값을 구할 수 있는 식을 쓸지 고민이 많이 되었다. 결국 쉽게 가기 위해 근사적으로 구성하는 것이 좋겠다 생각 하여 최대한 식들을 간단하게 구성하였다. Numerical analysis가 정밀도와 계산속도 등 고려해야 할 옵션이 많아 생각보다 굉장히 어려운 문제이며 공부해보면 재미있는 주제가 될 수 있겠다고 생각하였다.
2. 
