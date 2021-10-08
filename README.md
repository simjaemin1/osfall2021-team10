### OSFALL2021 TEAM10 PROJ1


## 0. How to build kernel

내용

## 1. High level implementation
#### 1.1 System call implementation
#### 1.2 copy_from_user, copy_to_user
#### 1.3 Error handling, locks and others
#### 1.4 task_struct & doubly linked list
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



