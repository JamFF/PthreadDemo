# PthreadDemo
Linux下Pthread基本使用

POSIX线程（POSIX threads），简称pthreads，是线程的POSIX标准。该标准定义了创建和操纵线程的一整套API。在类Unix操作系统（Unix、Linux、Mac OS X等）中，都使用 `pthreads` 作为操作系统的线程。

## pthread函数库的安装

`pthread` 不是Linux下的默认的库，所以在Linux环境下，有的有些系统没有安装有 `pthread`，所以在编译有关多线程程序时，也就是在程序链接的时候，无法找到 `pthread` 库中函数的入口地址，于是链接会失败。所以在此我们必须先安装好 `pthread` 库。安装过程很简单使用下面命令即可。
```
sudo apt-get install manpages-posix-dev
```
然后再用 `man -k pthread` 就可以查找到当前 `manual` 中关于 `pthread` 的手册，按 `q` 可退出。

## 开启线程

#### pthread_create函数

```c
#include <stdio.h>
#include <pthread.h>

void* thr_fun(void* arg){
    printf("new thread\n");
    char* num = (char*)arg;
    int i;
    for(i = 0; i < 10; i++){
        printf("%s thread, i:%d\n", num, i);
    }
}

void main(){
    printf("main thread\n");
    // 线程id
    pthread_t tid;
    // 参数2：线程的属性，NULL默认属性
    // 参数3：线程创建之后执行的函数
    // 参数4：thr_fun函数接受的参数
    pthread_create(&tid, NULL, thr_fun ,"arg参数");
}
```

#### 编译文件

编译 `.c` 文件，可直接生成可执行文件，省略中间 `.o` 文件的步骤。
```shell
gcc 01.c -o 01
```
报错
```
/tmp/ccC0AheU.o: In function `main':
01.c:(.text+0x7b): undefined reference to `pthread_create'
collect2: error: ld returned 1 exit status
```
使用 `pthread` 需要在编译时需要添加 `-lpthread`，因为 `pthread` 库不是 Linux 系统默认的库，连接时需要使用静态库 `libpthread.a`，所以在编译的时候需要链接该库，重新编译，可生成可执行文件01。
```shell
gcc 01.c -o 01 -lpthread
```

#### pthread_join函数

执行01文件，只输出了 `main thread`。

* 由于创建新线程后，main函数立马结束，程序退出，所以可以在main结束前 `sleep`。

  ```c
  #include <stdio.h>
  #include <pthread.h>
  #include <unistd.h>

  void* thr_fun(void* arg){
      printf("new ithread\n");
      char* num = (char*)arg;
      int i;
      for(i = 0; i < 10; i++){
          printf("%s thread, i:%d\n", num, i);
      }
  }

  void main(){
      printf("main thread\n");
      // 线程id
      pthread_t tid;
      // 参数2：线程的属性，NULL默认属性
      // 参数3：线程创建之后执行的函数
      // 参数4：thr_fun函数接受的参数
      pthread_create(&tid, NULL, thr_fun ,"arg参数");
      sleep(1);
  }
  ```
  打印结果
  ```shell
  main thread
  new ithread
  arg参数 thread, i:0
  arg参数 thread, i:1
  arg参数 thread, i:2
  arg参数 thread, i:3
  arg参数 thread, i:4
  arg参数 thread, i:5
  arg参数 thread, i:6
  arg参数 thread, i:7
  arg参数 thread, i:8
  arg参数 thread, i:9
  ```

* 但是这种方式并不可控，应该使用 `pthread_join` 函数，它会阻塞等待指定id的线程结束，执行结果是一样的。

  ```c
  #include <stdio.h>
  #include <pthread.h>

  void* thr_fun(void* arg){
      printf("new ithread\n");
      char* num = (char*)arg;
      int i;
      for(i = 0; i < 10; i++){
          printf("%s thread, i:%d\n", num, i);
      }
  }

  void main(){
      printf("main thread\n");
      // 线程id
      pthread_t tid;
      // 参数2：线程的属性，NULL默认属性
      // 参数3：线程创建之后执行的函数
      // 参数4：thr_fun函数接受的参数
      pthread_create(&tid, NULL, thr_fun ,"arg参数");
      pthread_join(tid, NULL);
  }
  ```

* `pthread_join` 第二个参数是thr_fun退出时的参数，`return` 的返回值或者 `pthread_exit` 的参数。
  ```c
  #include <stdio.h>
  #include <pthread.h>

  void* thr_fun(void* arg){
      printf("new ithread\n");
      char* num = (char*)arg;
      int i;
      for(i = 0; i < 10; i++){
          printf("%s thread, i:%d\n", num, i);
          if(i == 5){
              // 线程退出（自杀）
              pthread_exit((void*)5);
              // 他杀：int pthread_cancel(pthread_t thread);
          }
      }
      return (void*)1;
  }

  void main(){
      printf("main thread\n");
      // 线程id
      pthread_t tid;
      // 参数2：线程的属性，NULL默认属性
      // 参数3：线程创建之后执行的函数
      // 参数4：thr_fun函数接受的参数
      pthread_create(&tid, NULL, thr_fun ,"arg参数");
      void* rval;
      // 等待tid线程结束
      // 参数2：thr_fun退出时传入的参数，return或者pthread_exit
      pthread_join(tid, &rval);
      printf("rval:%s\n", (int)rval);
  }
  ```

* `thr_fun` 退出时，不调用 `return` 和 `pthread_exit`，打印rval是没有意义的，甚至还会报错。

## 线程互斥

当两个线程同时处理
```c
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int i = 0;

void* thr_fun(void* arg){
    char* num = (char*)arg;
    for(; i < 5; i++){
        printf("%s thread, i:%d\n", num, i);
        sleep(1);
    }
    i = 0;
}

void main(){
    pthread_t tid1, tid2;
    pthread_create(&tid1, NULL, thr_fun, "No1");
    pthread_create(&tid2, NULL, thr_fun, "No2");

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
}
```
打印结果
```shell
No2 thread, i:0
No1 thread, i:0
No2 thread, i:1
No1 thread, i:2
No2 thread, i:3
No1 thread, i:4
No1 thread, i:1
No1 thread, i:2
No1 thread, i:3
No1 thread, i:4
```
我们的目标是，让两个线程互不影响，让一个线程先做完，再让另一个线程去做。
使用互斥锁解决问题。
```c
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int i = 0;
// 互斥锁
pthread_mutex_t mutex;

void* thr_fun(void* arg){
    // 加锁
    pthread_mutex_lock(&mutex);
    char* num = (char*)arg;
    for(; i < 5; i++){
        printf("%s thread, i:%d\n", num, i);
        sleep(1);
    }
    i = 0;
    // 解锁
    pthread_mutex_unlock(&mutex);
}

void main(){
    pthread_t tid1, tid2;
    // 初始化互斥锁
    pthread_mutex_init(&mutex, NULL);

    pthread_create(&tid1, NULL, thr_fun, "No1");
    pthread_create(&tid2, NULL, thr_fun, "No2");

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    // 销毁互斥锁
    pthread_mutex_destroy(&mutex);
}
```
打印结果
```shell
No2 thread, i:0
No2 thread, i:1
No2 thread, i:2
No2 thread, i:3
No2 thread, i:4
No1 thread, i:0
No1 thread, i:1
No1 thread, i:2
No1 thread, i:3
No1 thread, i:4
```

## 条件变量

* 生产等于消费
  ```c
  #include <stdio.h>
  #include <pthread.h>
  #include <unistd.h>

  // 产品队列
  int ready = 0;

  // 互斥锁
  pthread_mutex_t mutex;
  // 条件变量
  pthread_cond_t has_product;

  // 生产
  void* producer(void* arg){
      char* num = arg;
      // 条件变量
      for(;;){
          pthread_mutex_lock(&mutex);
          // 往队列中添加产品
          ready++;
          printf("producer %s, produce product\n", num);
          // 通知消费者，有新的产品可以消费了
          pthread_cond_signal(&has_product);
          printf("producer %s, signal\n", num);
          pthread_mutex_unlock(&mutex);
          sleep(1);
      }
  }

  // 消费者
  void* consumer(void* arg){
      char* num = arg;
      for(;;){
          pthread_mutex_lock(&mutex);
          // 不能使用if，因为存在spurious wakeup ‘虚假唤醒’
          while(ready == 0){
              // 没有产品，继续等待
              pthread_cond_wait(&has_product, &mutex);
              printf("consumer %s, wait\n", num);
          }
          // 有产品，消费产品
          ready--;
          printf("consumer %s, consume produce\n", num);
          pthread_mutex_unlock(&mutex);
          sleep(1);
      }
  }

  void main(){
      pthread_t tid_p, tid_c;
      // 初始化互斥锁和条件变量
      pthread_mutex_init(&mutex, NULL);
      pthread_cond_init(&has_product, NULL);

      // 生产者线程
      pthread_create(&tid_p, NULL, producer, "1");

      // 消费者线程
      pthread_create(&tid_c, NULL, consumer, "2");

      // 等待
      pthread_join(tid_p, NULL);
      pthread_join(tid_c, NULL);

      pthread_mutex_destroy(&mutex);
      pthread_cond_destroy(&has_product);
  }
  ```
  打印结果
  ```shell
  producer 1, produce product
  producer 1, signal
  consumer 2, wait
  consumer 2, consume produce
  producer 1, produce product
  producer 1, signal
  consumer 2, wait
  consumer 2, consume produce
  producer 1, produce product
  producer 1, signal
  consumer 2, wait
  consumer 2, consume produce
  ```

* 生产大于消费
  ```c
  #include <stdio.h>
  #include <pthread.h>
  #include <unistd.h>

  // 消费者数量
  #define CONSUMER_NUM 1
  // 生产者数量
  #define PRODUCER_NUM 2

  // 线程id数组
  pthread_t pids[CONSUMER_NUM + PRODUCER_NUM];

  // 产品队列
  int ready = 0;

  // 互斥锁
  pthread_mutex_t mutex;
  // 条件变量
  pthread_cond_t has_product;

  // 生产
  void* producer(void* arg){
      int num = (int)arg;
      // 条件变量
      for(;;){
          pthread_mutex_lock(&mutex);
          // 往队列中添加产品
          ready++;
          printf("producer %d, produce product\n", num);
          // 通知消费者，有新的产品可以消费了
          pthread_cond_signal(&has_product);
          printf("producer %d, signal\n", num);
          pthread_mutex_unlock(&mutex);
          sleep(1);
      }
  }

  // 消费者
  void* consumer(void* arg){
      int num = (int)arg;
      for(;;){
          pthread_mutex_lock(&mutex);
          // 不能使用if，因为存在spurious wakeup ‘虚假唤醒’
          while(ready == 0){
              // 没有产品，继续等待
              // 1.阻塞等待has_product被唤醒
              // 2.释放互斥锁，pthread_mutex_unlock
              // 3.被唤醒时，解除阻塞，重新申请获得互斥锁pthread_mutex_lock
              pthread_cond_wait(&has_product, &mutex);
              printf("consumer %d, wait\n", num);
          }
          // 有产品，消费产品
          ready--;
          printf("consumer %d, consume produce\n", num);
          pthread_mutex_unlock(&mutex);
          sleep(1);
      }
  }
  void main(){
      // 初始化互斥锁和条件变量
      pthread_mutex_init(&mutex, NULL);
      pthread_cond_init(&has_product, NULL);

      int i = 0;
      // 生产者线程
      for(; i < PRODUCER_NUM; i++){
          printf("producer %d\n", i);
          pthread_create(&pids[i], NULL, producer, (void*)i);
      }

      // 消费者线程
      for(; i < CONSUMER_NUM + PRODUCER_NUM; i++){
          printf("consumer %d\n", i);
          pthread_create(&pids[i], NULL, consumer, (void*)i);
      }

      // 等待
      for(i = 0; i < CONSUMER_NUM + PRODUCER_NUM; i++){
          pthread_join(pids[i], NULL);
      }

      // 销毁互斥锁和条件变量
      pthread_mutex_destroy(&mutex);
      pthread_cond_destroy(&has_product);
  }
  ```
  打印结果
  ```shell
  producer 0
  producer 1
  consumer 2
  producer 1, produce product
  producer 1, signal
  consumer 2, wait
  consumer 2, consume produce
  producer 0, produce product
  producer 0, signal
  consumer 2, consume produce
  producer 1, produce product
  producer 1, signal
  producer 0, produce product
  producer 0, signal
  consumer 2, consume produce
  producer 1, produce product
  producer 1, signal
  producer 0, produce product
  producer 0, signal
  ```
* 生产小于消费
  ```c
  // 消费者数量
  #define CONSUMER_NUM 2
  // 生产者数量
  #define PRODUCER_NUM 1
  ```
  打印结果
  ```shell
  producer 0
  consumer 1
  consumer 2
  producer 0, produce product
  producer 0, signal
  consumer 2, wait
  consumer 2, consume produce
  producer 0, produce product
  producer 0, signal
  consumer 1, wait
  consumer 1, consume produce
  producer 0, produce product
  producer 0, signal
  consumer 2, wait
  consumer 2, consume produce
  producer 0, produce product
  producer 0, signal
  consumer 1, wait
  consumer 1, consume produce
  ```

## 项目应用

## 参考

[pthread/Linux 简单多线程编程](https://blog.csdn.net/always2015/article/details/44351501)
[面试常考点：虚假唤醒 Spurious wakeup](https://blog.csdn.net/fycy2010/article/details/47186887)