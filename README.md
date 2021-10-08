### OSFALL2021 TEAM10 PROJ1


## 0. How to build kernel0
먼저 파일을 다 받은 후, kernel image 디렉토리를 커널 디렉토리와 같은 디렉토리에 만든다. 그리고 다음과 같은 명령어를 차례대로 입력한다. 컴파일 과정은 build-rpi3-arm64.sh에 의해 이루어지며 이 파일을 실행하면 make 옵션을 자동을로 설정하여 커널 소스들을 컴파일 해준다. scripts 디렉토리에 있는 mkbootimg_rpi3.sh를 실행하면 이미지 파일 boot.img와 modules.img가 생긴다. 이들을 kernel image 디렉토리로 옮고 압축 파일을 이 디렉토리에 해제하여 나머지 이미지 파일들도 얻을 수 있다.
```bash
cd <kernel path>
./build-rpi3-arm64.sh
sudo ./scripts/mkbootimg_rpi3.sh
mv boot.img modules.img <kernel image path>
tar xvzf tizen-unified_20181024.1_iot-headless-2parts-armv7l-rpi3.tar.gz -C <kernel image path>
```
qemu.sh를 kernel 디렉토리에 놓고 다음과 같이 실행하면 tizen kernel이 실행된다.
```bash
cd <kernel path>
sudo ./qemu.sh
```
추가로 현재 linux 운영체제에서 tizen kernel source를 이용하여 컴파일 하고 싶은 경우 `arm-linux-gnueabi-gcc -I/include test_ptree.c -o test
`와 같이 컴파일 옵션을 주어 할 수 있다. 이는 test 디렉토리의 `Makefile`을 이용하여 구현하였다. 현재 linux 운영체제에 있는 파일을 옯길 경우 다음과 같이 `rootfs.img` 를 `mnt_dir`에 마운트한 후 `mnt_dir/root` 에 원하는 파일을 이동시키면 된다. 이후 커널 이미지를 `qemu.sh` 로 실행하면 `root` 디렉토리에 해당 파일을 확인할 수 있다.
```bash
mkdir mnt_dir
sudo mount rootfs.img <mnt_dir path>
cp <file> <mnt_dir path>/root/
```


## 1. High level implementation
#### 1.1 System call implementation

#### 1.2 copy_from_user, copy_to_user

#### 1.3 Error handling, locks and others

#### 1.4 task_struct & doubly linked list
linux는 `task_strcut`구조체를 이용하여 프로세스를 관리한다. `task_struct`는 같은 부모 process를 둔 자매끼리 doubly linked list 구조로 구현이 되어 있다. 이 doubly linked list에는 head가 존재하며 이는 데이터를 가지지 않는 dummy node이다. 이 list에는 특이하게 노드가 data 안에 저장이 되는데 `list_head children`은 자녀들로 이루어진 linked list의 head를 가리키며 자녀들로 이루어진 list의 각각의 노드들은 `list_head sibling`을 가리킨다. sibling을 이용하여 해당 프로세스의 task_struct 구조체에 접근하는 것이 큰 과제중 하나였는데, 컴파일 단계에서 sibling의 offset이 고정된다는 점을 이용하여 task_struct에 접근할 수 있었다. 따라서 자식 프로세스의 task_struct에 접근하기 위해서는 `(struct task_struct*)(task->children.next-(struct task_struct*)0->sibling)` 과 같은 과정을 통해 할 수 있는데 이는 linux/list.h 에 list_entry라는 매크로를 통해 구현되어 있었다. 이 외에도 다양한 매크로 함수, 인라인 함수가 있었고 다음과 같은 함수들을 사용하였다.
* list_entry(ptr, type, member)
* list_first_entry(ptr, type, member)
* list_next_entry(pos, member)
* list_empty(head)
* list_is_last(list, head)

#### 1.5 tree traversal

#### 1.6 store_prinfo, print_prinfo

#### 1.7 test file


## 2. Process tree
#### 2.1 test result
#### 2.2 tree investigation


## 3. Lesson learned
#### 3-1. 
계속 커널 버퍼 오버플로우가 일어나서 왜 일어났는지 알아내기 위해 많은 시간을 소요하였다. 여기저기에 printk를 넣어보고 코드를 지웠다가 썼다가를 반복하였다. 알고보니 copy_to_user에서이미 포인터였던 buf_k를 &buf_k로 잘못 써서 발생한 일이었다. (8/1920)이며 버퍼가 오버플로우 되었다는 메세지를 보고 왜 버퍼 크기가 8밖에 안되지? 라는 생각을 하였는데 생각해보니 주소는 integer이고 8바이트의 저장공간을 차지하는데 이를 저장하는 공간을 printfo 20개로 이루어진 1920바이트짜리 array가 저장된 공간에 할당하려고 해서 그런 일이 발생한 것 같다.
#### 3-2. 
uid를 복사하는 과정에서 자꾸 특정 aggregated라는 말이 들어간 에러 메세지가 출력되었다. 알고보니 구조체를 int64_t로 강제 형 변환해서 난 문제였다. task->cred->uid가 정수형 자료형인 줄 알았으나 구조체였다. 결국 task->cred->uid->val로 고치니 해결이 되었다. 해당 에러 메세지를 다시 보게 되면 이제 문제를 잘 해결할 수 있을 것 같다.
#### 3-3. 
ctags를 활용하는 방법에 익숙해졌다. linux 커널의 여러가지 구조체와 함수들을 찾아보면서 ctags를 활용하였는데 나름 도움이 되었던 것 같다. 다음에도 프로젝트 코드를 뜯어볼 일이 생긴다면 잘 활용할 수 있을 것 같다.
#### 3-4. 
doubly linked list 구조에 대해 자세히 알 수 있었다. 특히 구조체 내부에 있는 list_head를 통해 현재 구조체에 접근할 수 있도록 linked list를 만든 것은 처음 보았는데, 매우 신기하고 어려웠다. parent의 children이 sibling을 가리키기 때문에 sibling의 offset을 빼주면 된다는 점을 이해하고 나니까 list 함수들이 이해되기 시작하였다. 다만 이를 이해하고도 first children을 출력하는 구현을 하기까지 고생을 하였는데, list의 head가 더미 구조체라는 사실을 몰라서 고생을 하였다. 이는 list_first_children 함수의 코드를 보고나서야 알 수 있었다.    
#### 3-5.
init_task가 swapper를 가리킨다는 사실을 몰라서 처음에는 현재 process부터 맨 위까지 거슬러 올라가는get_top_process라는 함수를 만들어서 직접 찾았는데, init_task라는 간편한 방법이 있음을 알게 되었다.



