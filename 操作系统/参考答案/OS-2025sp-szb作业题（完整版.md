---
title: OS_2025sp_szb作业题
author: HeZzz
published: true
date: 2025-05-09 09:50:11
tags:
    - OS
categories: 学习
---
> 本文连载于[HeZzz的博客 之 OS_2025sp_szb作业题](https://blog.hezi.space/2025/05/09/OS-2025sp-szb%E4%BD%9C%E4%B8%9A%E9%A2%98/)。
>
> 关于操作系统还有[OS_2025sp考点](https://blog.hezi.space/2025/05/08/OS2025sp%E8%80%83%E7%82%B9/)。
>
> szb 的作业题，来源于[计算机速通之家 | QQ 群号：468081841](https://qm.qq.com/q/ojSHMvHG5a)，把题目和答案喂给 DeepSeek 后整出来个文档。

🙇‍♂️🙇‍♂️🙇‍♂️时间仓促，有不足之处烦请及时告知。

题目:

- [第一次作业(hw1.pdf)](/pdf/os25sp/hw1.pdf)
- [第二次作业(hw2.pdf)](/pdf/os25sp/hw2.pdf)

答案:

- [答案(ans.pdf)](/pdf/os25sp/ans.pdf)

---

## hw1

### **第一题：独木桥同步问题详解**

> 前两道都是 PV 操作，不熟悉的可以看[王道计算机考研-操作系统视频](https://www.bilibili.com/video/BV1YE411D7nH/?share_source=copy_web&vd_source=ece2a9c84bf4c011ecb77b7f31228f25)。

#### **1. 每次只允许一人过桥**

- **问题核心**：确保桥的互斥访问，任何时刻仅有一个行人通过。
- **实现方法**：
  - 定义一个**互斥信号量 `mutex`**，初始值为1。
  - **过桥流程**：
    1. 行人到达桥头时执行 `P(mutex)`（申请锁）：
       - 若 `mutex=1`，则获取锁成功，允许过桥。
       - 若 `mutex=0`，则阻塞等待。
    2. 行人过桥后执行 `V(mutex)`（释放锁），唤醒其他等待的行人。
- **伪代码示例**：
  ```c
  semaphore mutex = 1;
  cobegin
  process traveler() {
      P(mutex);   // 申请锁
      过桥;
      V(mutex);   // 释放锁
  }
  coend
  ```
- **关键点**：
  通过简单的互斥信号量实现“独占式”过桥，逻辑简单但效率较低。

---

#### **2. 同方向可多人，反方向需等待**

- **问题核心**：允许同方向行人批量通过，反方向行人需等待桥空闲。
- **实现方法**：
  - **定义资源**：
    - `numE2W`（东向西人数）、`numW2E`（西向东人数）：统计当前方向的行人数量。
    - `mutexE2W` 和 `mutexW2E`：保护 `numE2W` 和 `numW2E` 的互斥信号量。
    - `mutex`：方向锁，确保同方向优先占用桥。
  - **过桥流程**（以东向西为例）：
    1. 行人到达时执行 `P(mutexE2W)`，修改 `numE2W`：
       - 若 `numE2W == 0`（当前方向无行人），执行 `P(mutex)` 占用桥。
    2. `numE2W++`，释放 `mutexE2W`，开始过桥。
    3. 过桥后再次执行 `P(mutexE2W)`，修改 `numE2W`：
       - 若 `numE2W == 0`（当前方向无后续行人），执行 `V(mutex)` 释放桥。
    4. 释放 `mutexE2W`。
  - **伪代码示例**（东向西行人）：
    ```c
    semaphore mutex = 1, mutexE2W = 1, mutexW2E = 1;
    int numE2W = 0, numW2E = 0;
    cobegin
    process travelerE2W() {
        P(mutexE2W);       // 保护计数器
        numE2W++;
        if (numE2W == 1) {
            P(mutex);      // 首次占用桥
        }
        V(mutexE2W);
        过桥;
        P(mutexE2W);
        numE2W--;
        if (numE2W == 0) {
            V(mutex);      // 最后一人释放桥
        }
        V(mutexE2W);
    }
    coend
    ```
- **关键点**：
  - **方向锁机制**：首个行人占用桥，后续同方向行人无需等待。
  - **计数器保护**：通过 `mutexE2W` 和 `mutexW2E` 避免并发修改计数器的竞态条件。

---

#### **3. 东向西可多人，西向东仅单人**

- **问题核心**：东向西允许多个行人同时过桥，西向东必须严格互斥。
- **实现方法**：
  - **东向西行人**：与情况2相同，共享 `numE2W` 计数器，首个行人占用桥。
  - **西向东行人**：直接使用互斥锁 `mutex`，每次仅允许一人过桥。
  - **伪代码示例**（西向东行人）：
    ```c
    process travelerW2E() {
        P(mutex);          // 直接互斥占用桥
        过桥;
        V(mutex);
    }
    ```
- **关键点**：
  - **不对称设计**：东向西方向通过计数器实现批量通过，而西向东方向强制互斥。
  - **避免死锁**：两种方向锁独立，不会因互相等待导致死锁。

---

#### **总结**

- **信号量选择**：
  - `mutex`：控制桥的占用状态。
  - `mutexE2W`/`mutexW2E`：保护方向计数器的原子性。
- **同步逻辑**：
  - **计数器增减**：必须在保护信号量内完成。
  - **方向锁释放**：仅当计数器归零时释放，确保反方向行人能及时获得桥使用权。
- **扩展思考**：
  - 若东西方向同时有人到达，可能引发短暂竞争，但通过锁机制确保最终一致性。
  - 可通过优化信号量设计（如读写锁）进一步提升并发效率，但需权衡复杂度。

---

### 第二题：缓冲区同步与互斥的详细解析

#### **问题背景**

三个进程 `P1`、`P2`、`P3` 共享一个包含 `N` 个单元的缓冲区：

- **P1**：生成正整数并放入缓冲区。
- **P2**：从缓冲区取出奇数并统计。
- **P3**：从缓冲区取出偶数并统计。

需用信号量机制实现以下功能：

1. **互斥**：同一时刻只能有一个进程操作缓冲区。
2. **同步**：
   - `P1` 放入数据后，通知 `P2` 或 `P3` 取出。
   - `P2` 和 `P3` 只能取出符合条件的数据（奇数或偶数）。

---

#### **信号量定义与作用**

| 信号量    | 初始值 | 作用                                                 |
| --------- | ------ | ---------------------------------------------------- |
| `mutex` | 1      | 互斥访问缓冲区，确保同一时间只有一个进程操作缓冲区。 |
| `empty` | N      | 表示缓冲区中空单元的数量，控制 `P1` 的写入条件。   |
| `even`  | 0      | 表示缓冲区中偶数的数量，控制 `P3` 的读取条件。     |
| `odd`   | 0      | 表示缓冲区中奇数的数量，控制 `P2` 的读取条件。     |

---

#### **伪代码解析**

##### **P1（生产者）**

```c
while (True){
    int x = produce()         // 生成一个正整数
    P(empty);              // 申请空缓冲区（若无空位则阻塞）
    P(mutex);              // 申请互斥锁
    put(x);                // 将 x 放入缓冲区
    V(mutex);              // 释放互斥锁
    if x % 2 == 0
        V(even);           // 通知 P3 有偶数可用
    else
        V(odd);            // 通知 P2 有奇数可用
}
```

- **关键点**：
  - `P(empty)` 先于 `P(mutex)`：避免死锁（若先申请 `mutex` 后申请 `empty`，当缓冲区满时，`P1` 会持有 `mutex` 并阻塞，其他进程无法操作缓冲区）。
  - 根据奇偶性触发对应的信号量（`V(even)` 或 `V(odd)`），通知消费者进程。

---

##### **P2（奇数消费者）**

```c
while True{
    P(odd);                // 等待奇数可用（若无则阻塞）
    P(mutex);              // 申请互斥锁
    getodd();              // 从缓冲区取出奇数
    V(mutex);              // 释放互斥锁
    countodd();            // 统计奇数
    V(empty);              // 释放一个空单元
}
```

- **关键点**：
  - `P(odd)` 先于 `P(mutex)`：若先申请 `mutex`，可能因无数据可用而持有锁阻塞，导致死锁。
  - `V(empty)` 表示取出数据后，空单元数量增加。

---

##### **P3（偶数消费者）**

```c
while True{
    P(even);               // 等待偶数可用（若无则阻塞）
    P(mutex);              // 申请互斥锁
    geteven();             // 从缓冲区取出偶数
    V(mutex);              // 释放互斥锁
    counterven();          // 统计偶数
    V(empty);              // 释放一个空单元
}
```

- **逻辑与 P2 对称**，仅信号量 `even` 和操作函数不同。

---

#### **同步与互斥流程**

1. **互斥**：`P1`、`P2`、`P3` 通过 `mutex` 保证对缓冲区的互斥访问。
2. **同步**：
   - **生产者-消费者同步**：`P1` 通过 `V(even)` 或 `V(odd)` 通知消费者数据可用。
   - **缓冲区容量控制**：`empty` 信号量确保 `P1` 不会在缓冲区满时写入。

---

#### **答案验证与边界场景**

1. **缓冲区满时**：
   - `P1` 因 `P(empty)` 阻塞，无法继续写入。
   - `P2` 或 `P3` 取出数据后，`V(empty)` 唤醒 `P1`。
2. **缓冲区空时**：
   - `P2` 或 `P3` 因 `P(odd)/P(even)` 阻塞，无法读取。
   - `P1` 写入数据后，`V(odd)/V(even)` 唤醒对应的消费者。
3. **奇偶数据不均衡时**：
   - 若缓冲区中只有奇数，`P3` 会因 `P(even)` 阻塞，直到 `P1` 写入偶数。

---

#### **总结**

通过 `mutex`、`empty`、`even`、`odd` 四个信号量的协同，实现了：

1. **互斥**：缓冲区操作的安全性。
2. **同步**：
   - 生产者与消费者的协调。
   - 奇偶数据分类通知。
   - 缓冲区容量动态管理。

---

### 第三题：进程调度算法

> 五个调度算法都得看一眼吧（）

- 先来先服务（FCFS）
- 短作业优先（SJF）
- 最短剩余时间优先（SRTF）
- 高响应比优先（HRRN）
- 优先权调度

#### **题目描述**

- **4个进程的到达时间与所需CPU时间**：

  | 进程 | 到达时间 | 所需 CPU 时间 |
  | ---- | -------- | ------------- |
  | P1   | 0        | 10            |
  | P2   | 2        | 4             |
  | P3   | 4        | 10            |
  | P4   | 6        | 5             |
- **要求**：

  1. **短进程优先（SPF，非抢占）**：分析执行顺序，计算平均周转时间和带权平均周转时间。
  2. **最短剩余时间优先（SRTF，抢占）**：分析执行顺序，计算平均周转时间和带权平均周转时间。

---

#### **1. 短进程优先（SPF，非抢占）**

##### **执行顺序**

1. **0时刻**：只有P1到达，开始执行P1（运行时间10）。
2. **P1执行完成（时刻10）**：
   - 此时已到达的进程有P2（到达时间2）、P3（到达时间4）、P4（到达时间6）。
   - 选择剩余时间最短的进程：P2（4）、P4（5）、P3（10）。
   - **执行顺序**：P2（4）→ P4（5）→ P3（10）。

##### **时间轴**

- **P1**：0 → 10（结束时间10）
- **P2**：10 → 14（结束时间14）
- **P4**：14 → 19（结束时间19）
- **P3**：19 → 29（结束时间29）

##### **计算指标**

| 进程 | 到达时间 | 结束时间 | 周转时间 | 带权周转时间 |
| ---- | -------- | -------- | -------- | ------------ |
| P1   | 0        | 10       | 10       | 10/10 = 1    |
| P2   | 2        | 14       | 12       | 12/4 = 3     |
| P3   | 4        | 29       | 25       | 25/10 = 2.5  |
| P4   | 6        | 19       | 13       | 13/5 = 2.6   |

- **平均周转时间**：\( \frac{10+12+25+13}{4} = 15 \)
- **带权平均周转时间**：\( \frac{1+3+2.5+2.6}{4} = 2.275 \)

---

#### **2. 最短剩余时间优先（SRTF，抢占）**

##### **执行顺序**

1. **0时刻**：P1开始执行。
2. **时刻2**：P2到达，剩余时间（P1剩余8，P2剩余4）。
   - **P1被抢占**，执行P2（剩余时间更短）。
3. **时刻6**：P2完成。
   - 已到达进程：P1（剩余8）、P3（到达时间4，剩余10）、P4（到达时间6，剩余5）。
   - 选择剩余时间最短的进程：P4（5）→ P1（8）→ P3（10）。
4. **时刻11**：P4完成。
   - 剩余进程：P1（剩余8）、P3（剩余10）。
   - 继续执行P1（剩余8）。
5. **时刻19**：P1完成。
   - 最后执行P3（剩余10）。
6. **时刻29**：P3完成。

##### **时间轴**

- **P1**：0→2（被抢占）→11→19（总运行时间10）
- **P2**：2→6（结束时间6）
- **P4**：6→11（结束时间11）
- **P3**：19→29（结束时间29）

##### **计算指标**

| 进程 | 到达时间 | 结束时间 | 周转时间 | 带权周转时间 |
| ---- | -------- | -------- | -------- | ------------ |
| P1   | 0        | 19       | 19       | 19/10 = 1.9  |
| P2   | 2        | 6        | 4        | 4/4 = 1      |
| P3   | 4        | 29       | 25       | 25/10 = 2.5  |
| P4   | 6        | 11       | 5        | 5/5 = 1      |

- **平均周转时间**：\( \frac{19+4+25+5}{4} = 13.25 \)
- **带权平均周转时间**：\( \frac{1.9+1+2.5+1}{4} = 1.6 \)

---

#### **关键点总结**

1. **短进程优先（SPF）**：

   - **非抢占**，一旦开始执行某进程，除非完成，否则不切换。
   - 执行顺序取决于进程到达时的剩余时间，优先选择最短的。
2. **最短剩余时间优先（SRTF）**：

   - **抢占式**，当新进程到达时，比较剩余时间，选择最短的立即执行。
   - 需动态更新剩余时间，可能导致频繁切换。
3. **周转时间** = 结束时间 - 到达时间
4. **带权周转时间** = 周转时间 / 服务时间
5. **抢占式算法的优势**：减少平均等待时间，但可能增加调度开销。

通过对比两种算法，SRTF的平均周转时间（13.25）优于SPF（15），体现了抢占式调度在响应短进程时的优势。

---

### 第四题：批处理系统调度的详细

#### **问题背景**

在一个具有两道作业的批处理系统中，作业调度采用**短作业优先（SJF）**算法，进程调度采用**基于优先数的抢占式调度算法**（优先数越小，优先级越高）。需分析作业序列的执行顺序、进入内存时间、结束时间，并计算平均周转时间。

---

#### **作业参数**

| 作业名 | 到达时间 | 估计运行时间 | 优先数 |
| ------ | -------- | ------------ | ------ |
| A      | 10:00    | 40分钟       | 5      |
| B      | 10:20    | 30分钟       | 3      |
| C      | 10:30    | 50分钟       | 4      |
| D      | 10:50    | 20分钟       | 6      |

---

#### **调度规则**

1. **作业调度（SJF）**：
   - 当内存有空位时，从后备队列中选择**运行时间最短**的作业调入内存。
2. **进程调度（优先数抢占）**：
   - 从内存中的作业中选择**优先数最小**的作业执行。
   - 若新到达的作业优先数更小，则**抢占当前作业**。

---

#### **执行过程与时间线**

##### **1. 初始时刻（10:00）**

- **作业A到达**，直接进入内存并开始执行（此时内存中仅A）。
- **内存状态**：`[A]`。

---

##### **2. B到达（10:20）**

- **作业B到达**，内存有空位（系统允许两道作业），B进入内存。
- **进程调度**：比较A（优先数5）和B（优先数3）。B优先级更高，**抢占A**。
- **执行顺序**：B开始执行，A被挂起。
- **内存状态**：`[A, B]`。

---

##### **3. B执行结束（10:50）**

- **B运行30分钟**（10:20–10:50），释放内存位置。
- **作业调度**：从后备队列中选择运行时间最短的作业。此时：
  - C（到达时间10:30，运行时间50分钟）。
  - D（到达时间10:50，运行时间20分钟）。
  - **选择D**（运行时间更短）。
- **内存状态**：`[A, D]`。
- **进程调度**：比较A（优先数5）和D（优先数6）。A优先级更高，**恢复执行**。

---

##### **4. A执行结束（11:10）**

- **A剩余运行时间20分钟**（总40分钟，已运行20分钟），从10:50执行到11:10结束。
- **内存状态**：`[D]`（释放一个位置）。
- **作业调度**：调入C（运行时间50分钟，但此时D已在内存）。
- **内存状态**：`[C, D]`。
- **进程调度**：比较C（优先数4）和D（优先数6）。C优先级更高，**开始执行C**。

---

##### **5. C执行结束（12:00）**

- **C运行50分钟**（11:10–12:00）。
- **内存状态**：`[D]`（释放一个位置）。
- **进程调度**：执行D（唯一在内存中的作业）。

---

##### **6. D执行结束（12:20）**

- **D运行20分钟**（12:00–12:20）。

---

#### **作业时间表**

| 作业名 | 进入内存时间 | 结束时间 | 周转时间                |
| ------ | ------------ | -------- | ----------------------- |
| A      | 10:00        | 11:10    | 70分钟（11:10 - 10:00） |
| B      | 10:20        | 10:50    | 30分钟（10:50 - 10:20） |
| C      | 11:10        | 12:00    | 90分钟（12:00 - 10:30） |
| D      | 10:50        | 12:20    | 90分钟（12:20 - 10:50） |

---

#### **关键逻辑验证**

1. **作业调度（SJF）触发时机**：
   - 仅在内存有空位时触发（如B结束后调入D，A结束后调入C）。
2. **进程抢占逻辑**：
   - B抢占A（优先数3 < 5），C抢占D（优先数4 < 6）。
3. **内存管理**：
   - 系统始终最多容纳两道作业（如B和A共存，C和D共存）。

---

#### **平均周转时间计算**

\[
\text{平均周转时间} = \frac{70 + 30 + 90 + 90}{4} = 70\ \text{分钟}
\]

---

#### **注意**

系统之所以在任意时刻内存中最多只维持两道作业，正是依据题目中“具有两道作业的批处理系统”这一设定：

1. **作业调度触发条件**

   * 只有当内存中已有作业数小于 2（即出现空闲位置）时，SJF 调度才会从后备队列中再选入一个作业。
   * 因此，每次调度只补入一个作业，直到内存正好装满两道作业或后备队列为空。
2. **进程调度作用范围**

   * 进程调度（基于优先数的抢占）始终只在**当前已在内存的那两道作业**之间进行决策与切换。
   * 新作业只有被作业调度选中并装入内存后，才有机会参与后续的抢占式执行。

换言之，系统的双槽（two-slot）内存限制决定了“每次 SJF 只选入一个作业”以及“CPU 调度只在这两道已装入的作业间进行”。

---

### 第五题：银行家算法详细解析

对银行家算法不熟悉的米娜桑可以先看[操作系统-银行家算法](https://www.bilibili.com/video/BV1rJ411p7au/?share_source=copy_web&vd_source=ece2a9c84bf4c011ecb77b7f31228f25)。

> 随便找的视频，若有更好的可以发群里 [计算机速通之家 | QQ 群号：468081841](https://qm.qq.com/q/ojSHMvHG5a) 并艾特 `9¾`

#### **1. T0时刻是否为安全状态？**

**步骤分析：**

1. **计算各进程剩余需求（最大需求 - 已分配）：**

   - **P1**：A=5-2=3，B=5-1=4，C=9-2=7
   - **P2**：A=5-4=1，B=3-0=3，C=6-2=4
   - **P3**：A=4-4=0，B=0-0=0，C=11-5=6
   - **P4**：A=4-2=2，B=2-0=2，C=5-4=1
   - **P5**：A=4-3=1，B=2-1=1，C=4-4=0
2. **初始可用资源**：A=2，B=3，C=3
3. **寻找安全序列：**

   - **第一步**：检查 **P3**（剩余需求A=0，B=0，C=6），但可用C=3 < 6，不满足。
   - **第二步**：检查 **P5**（剩余需求A=1，B=1，C=0），可用资源满足（A=2≥1，B=3≥1）。分配P5，释放其资源（A=3，B=1，C=4），可用资源变为 **A=5，B=4，C=7**。
   - **第三步**：检查 **P4**（剩余需求A=2，B=2，C=1），可用资源满足（A=5≥2，B=4≥2）。分配P4，释放其资源（A=2，B=0，C=4），可用资源变为 **A=7，B=4，C=11**。
   - **第四步**：检查 **P1**（剩余需求A=3，B=4，C=7），可用资源满足（A=7≥3，B=4≥4）。分配P1，释放其资源（A=2，B=1，C=2），可用资源变为 **A=9，B=5，C=13**。
   - **第五步**：检查 **P2**（剩余需求A=1，B=3，C=4），可用资源满足（A=9≥1，B=5≥3）。分配P2，释放其资源（A=4，B=0，C=2），可用资源进一步增加。
   - **最终安全序列**：**P5 → P4 → P1 → P2 → P3**（不唯一）。

**结论**：T0时刻是安全状态，存在安全序列。

---

#### **2. 进程P4请求资源（2,0,1），能否分配？**

**步骤分析：**

1. **验证请求合法性：**

   - P4的剩余需求：A=2，B=2，C=1 → 请求（2,0,1） ≤ 剩余需求。
   - 系统当前可用资源：A=2，B=3，C=3 → 请求 ≤ 可用资源。
2. **尝试分配并检查安全性：**

   - 分配后，可用资源变为：**A=0，B=3，C=2**。
   - P4的已分配资源变为：A=4，B=0，C=5，剩余需求：A=0，B=2，C=0。
   - **寻找安全序列：**
     - **第一步**：分配 **P4**（剩余需求B=2 ≤ 可用B=3），释放资源后可用资源变为 **A=4，B=3，C=7**。
     - **第二步**：分配 **P5**（剩余需求A=1，B=1），释放后可用资源变为 **A=7，B=4，C=11**。
     - **第三步**：分配 **P1**（剩余需求A=3，B=4，C=7），释放后可用资源变为 **A=9，B=5，C=13**。
     - **第四步**：分配 **P2**（剩余需求A=1，B=3，C=4），释放后可用资源进一步增加。
     - **安全序列**：**P4 → P5 → P1 → P2 → P3**。

**结论**：允许分配，存在安全序列。

---

#### **3. 在（2）的基础上，进程P1请求资源（0,2,0），能否分配？**

**步骤分析：**

1. **当前可用资源**（已分配P4请求后）：A=0，B=3，C=2。
2. **验证请求合法性：**

   - P1的剩余需求：A=3，B=4，C=7 → 请求（0,2,0） ≤ 剩余需求。
   - 可用资源：B=3 ≥ 2，其他资源足够。
3. **尝试分配并检查安全性：**

   - 分配后，可用资源变为：**A=0，B=1，C=2**。
   - P1的已分配资源变为：A=2，B=3，C=2，剩余需求：A=3，B=2，C=7。
   - **检查安全序列：**
     - 所有进程的剩余需求均无法被满足：
       - P3需要C=6 > 可用C=2；
       - P2需要A=1 > 可用A=0；
       - P5需要A=1 > 可用A=0。
   - **无安全序列**，系统进入不安全状态。

**结论**：拒绝分配，请求会导致系统不安全。

---

#### **最终答案**

1. **T0时刻是安全状态**，安全序列示例：P5 → P4 → P1 → P2 → P3。
2. **允许P4请求（2,0,1）**，存在安全序列（如P4 → P5 → P1 → P2 → P3）。
3. **拒绝P1请求（0,2,0）**，分配后系统将处于不安全状态。

---

## hw2

### 第1题：页表结构解析

---

#### **题目回顾**

一个32位地址的计算机系统使用二级页表，虚拟地址划分如下：

- 顶级页表占9位
- 二级页表占11位

**问题**：

1. 页面长度是多少？
2. 虚拟地址空间有多少个页面？

---

#### **答案与解析**

##### **1. 页面长度**

- **计算页内偏移量**：虚拟地址总长度为32位，扣除顶级页表（9位）和二级页表（11位），剩余部分为页内偏移量：`32 - 9 - 11 = 12 位`，因此，页面长度为：`2 ^ 12 = 4096 字节 = 4KB`
- **验证**：
  页内偏移量12位意味着每个页面可寻址4KB空间，这与现代操作系统的标准页面大小一致（如Linux默认4KB）。

---

##### **2. 虚拟地址空间的页面总数**

- **页号的总位数**：
  页号由顶级页表索引（9位）和二级页表索引（11位）组成，总位数为：
  `9 + 11 = 20 位`
  因此，虚拟地址空间的最大页面数为：
  `2 ^ 20 = 1 M 个页面`

---

#### **关键概念总结**

1. **二级页表的作用**：

   - 顶级页表（Page Directory）索引二级页表（Page Table），二级页表索引物理页框。
   - 分层设计减少页表内存占用（无需为未使用的虚拟地址分配二级页表）。
2. **地址划分的意义**：

   - **顶级页表位（9位）**：决定一级页表数量。
   - **二级页表位（11位）**：决定每个一级页表项能映射的二级页表数量。
   - **页内偏移（12位）**：决定页面大小。
3. **典型应用场景**：

   - 32位系统中，二级页表常用于平衡内存占用与地址映射效率。例如，x86架构的经典分页模式（10位顶级页表 + 10位二级页表 + 12位偏移）。

---

#### **常见疑问解答**

**为什么总页号位数是20位？**
顶级页表（9位）和二级页表（11位）共同确定一个物理页框，因此页号总位数为两者之和。每个页号对应唯一的物理页框，总页数为 `2^20`。

**如果页面长度改为8KB，地址划分会如何变化？**
页内偏移量需要13位（`2^13 = 8KB`），此时顶级页表和二级页表的位数之和为：
`32 - 13 = 19 位`
可能划分为9位 + 10位，或其他组合。

---

#### **结论**

通过分析虚拟地址的划分，可以得出：

1. **页面长度**：4KB
2. **虚拟地址空间页面数**：1M个
   这一结果与二级页表的设计逻辑完全吻合，体现了分页机制在内存管理中的核心作用。

---

### 第2题：虚拟地址转换

#### **一、题目条件**

1. **用户编程空间**：共 32 个页面，每页 1KB（即 1024 字节）。
2. **物理内存**：大小为 16KB，划分为 16 个物理块，每块 1KB。
3. **页表（部分）**：

    | 页号 | 物理块号 |
    | ---- | -------- |
    | 0    | 5        |
    | 1    | 10       |
    | 2    | 4        |
    | 3    | 7        |

4. **逻辑地址**：`0A5C(H)`（十六进制）

---

#### **二、地址结构分析**

* 页数为 32 → 页号需占 **5 位**（$2^5 = 32$）
* 每页大小为 1KB → 页内偏移需占 **10 位**（$2^{10} = 1024$）
* 因此逻辑地址总长度为 **5 + 10 = 15 位**

---

#### **三、逻辑地址解析**

* 原始逻辑地址：`0A5C(H)` = `0000 1010 0101 1100(B)`（16位）
* 舍弃最高冗余位后，取低 15 位：`000 1010 0101 1100`

  * 前 5 位：页号 = `00010(B)` = `2(₁₀)`
  * 后 10 位：页内偏移 = `1001011100(B)` = `604(₁₀)` = `25C(H)`

---

#### **四、查页表获取物理块号**

* 页号 2 对应的物理块号为 **4**

---

#### **五、计算物理地址**

##### **十六进制加法方式**

- **每块大小**：1KB = 2 ^ 10 字节
- **物理块号 4 的起始地址**：4 × 1KB = 2 ^ 12 = 1000H（十六进制）
- **偏移量**:
  25CH
- **最终物理地址**：
  1000H + 25CH = **125CH**

---

**计算说明**：

1. **块号转起始地址**：
   - 块号 4 × 每块大小 1024 = 4096（十进制） = 1000H。
2. **偏移量直接使用**：
   - 逻辑地址中的偏移量为 25CH。
3. **物理地址合成**：
   - 起始地址 1000H + 偏移量 25CH = 125CH。

**结论**：
物理地址为 **125CH**。

---

#### **六、验证与结论**

* **页号** = 2（已在页表中）
* **偏移量** = 604 ∈ \[0, 1023]（合法范围）
* **最终物理地址** = `125C(H)`

---

#### **七、常见问题解答**

| 问题                                      | 解答                                                                       |
| ----------------------------------------- | -------------------------------------------------------------------------- |
| **为何逻辑地址为 15 位？**          | 5 位页号 + 10 位偏移，共 15 位，题中地址为 16 位，需去除最高冗余位。       |
| **页表未包含页号 4\~31 如何处理？** | 若访问这些页，将触发缺页异常，操作系统需将页面调入内存。题目未涉及此情形。 |

---

### 第3题：页面置换算法详细解析

> DeepSeek 把自己绕进去了，这里只留下大概的解析，具体的缺页表格可以看 [ans.pdf（就是文章最开始的答案文件）](/pdf/ans.pdf)。
> 缺页这部分只考这三个算法，米娜桑不熟悉的找找视频看看书。
> 放个视频[（存档）操作系统页面置换算FIFO,OPT,LRU做题速成](https://www.bilibili.com/video/BV1et421K7d3/?share_source=copy_web&vd_source=ece2a9c84bf4c011ecb77b7f31228f25)。

#### **题目要求**

- **页面走向**：1, 2, 3, 4, 2, 3, 5, 6, 3, 1, 4, 6, 7, 5, 2, 4, 1, 3, 2
- **物理块数**：3个
- **算法**：FIFO、OPT、LRU
- **目标**：计算缺页中断次数和缺页率

---

#### **1. FIFO（先进先出）算法**

**核心思想**：替换最早进入内存的页面。**模拟过程**：

- 维护一个队列记录页面进入顺序，缺页时替换队首页面，新页面加入队尾。

**缺页次数** ：14次

**缺页率** ：14 /19

---

#### **2. OPT（最佳置换）算法**

**核心思想**：替换未来最长时间不会被访问的页面。**模拟过程**：

- 预知未来访问序列，选择最晚被访问的页面替换。

**缺页次数**：8次
**缺页率**：8 / 19

---

#### **3. LRU（最近最久未使用）算法**

**核心思想**：替换最近最久未被访问的页面。**模拟过程**：

- 维护访问顺序，每次访问后更新页面为“最近使用”，缺页时替换最久未使用的页面。

**缺页次数**：13次
**缺页率**：13 / 19

---

#### 总结

| 算法 | 缺页次数 | 缺页率  |
| ---- | -------- | ------- |
| FIFO | 14       | 14 / 19 |
| OPT  | 8        | 8 / 19  |
| LRU  | 13       | 13 / 19 |

**关键结论**：

1. **FIFO** 性能最差，因其无法适应访问模式的局部性。
2. **OPT** 是理论最优解，但需预知未来访问序列，实际不可行。
3. **LRU** 接近 OPT，但需要维护访问历史，开销较大。在此例中，由于存在周期性重复访问旧页面，LRU表现略逊于理论预期。

---

### 第4题：TLB与缺页中断的详细解析

#### **题目条件**

1. **系统参数**：

   - 页面大小：4KB（页内偏移占12位）。
   - 内存访问时间：100ns，TLB访问时间：10ns。
   - 缺页处理时间：108ns（包含更新TLB和页表的时间）。
   - 驻留集大小固定为2，采用LRU置换算法和局部淘汰策略。
   - TLB初始为空，且正常页表访问后不更新TLB（仅在缺页处理后更新）。
2. **页表初始状态**：

   | 页号 | 页框号 | 有效位 |
   | ---- | ------ | ------ |
   | 0    | 101H   | 1      |
   | 1    | --     | 0      |
   | 2    | 254H   | 1      |

---

#### **逻辑地址访问分析**

##### **1. 访问逻辑地址 `2362H`**

- **地址分解**：

  - 十六进制 `2362H` → 二进制 `0010 0011 0110 0010`。
  - **页号**：高4位 `0010`（十进制2）。
  - **页内偏移**：低12位 `362H`。
- **转换流程**：

  1. **访问TLB**：初始为空，未命中（耗时10ns）。
  2. **访问页表**：页号2有效，页框号为 `254H`（耗时100ns）。
  3. **访问物理地址**：（耗时100ns）。
  4. **总时间**：`10 + 100 + 100 = 210ns`。

---

##### **2. 访问逻辑地址 `1565H`**

- **地址分解**：

  - 十六进制 `1565H` → 二进制 `0001 0101 0110 0101`。
  - **页号**：高4位 `0001`（十进制1）。
  - **页内偏移**：低12位 `565H`。
- **转换流程**：

  1. **访问TLB**：未命中（耗时10ns）。
  2. **访问页表**：页号1无效（有效位为0），触发缺页中断。
  3. **缺页处理**：
     - 驻留集大小为2，当前驻留页为0和2（根据初始页表）。
     - 根据LRU算法，淘汰最近最少使用的页0（假设页0未被近期访问）。
     - 将页1装入原页0的页框 `101H`，更新页表（耗时108ns）。
     - **更新TLB**：缺页处理包含TLB更新，页1的映射被缓存。
  4. **重新执行指令**：
     - TLB命中页1（耗时10ns）。
     - 物理地址（耗时100ns）。
  5. **总时间**：`10（TLB） + 100（页表） + 108（缺页） + 100（物理地址） = 318ns`。

---

##### **3. 访问逻辑地址 `25A5H`**

- **地址分解**：

  - 十六进制 `25A5H` → 二进制 `0010 0101 1010 0101`。
  - **页号**：高4位 `0010`（十进制2）。
  - **页内偏移**：低12位 `5A5H`。
- **转换流程**：

  1. **访问TLB**：假设在缺页处理后，TLB中已缓存页2的映射（因题目未明确禁止缺页处理外的TLB更新）。
  2. **TLB命中**：直接获取页框号 `254H`（耗时10ns）。
  3. **访问物理地址**：物理地址（耗时100ns）。
  4. **总时间**：`10 + 100 = 110ns`。

#### **答案总结**

| 逻辑地址 | 转换步骤与时间（ns）                                                     |  |
| -------- | ------------------------------------------------------------------------ | - |
| 2362H    | TLB未命中 → 页表 → 物理地址（10+100+100 = 210）                        |  |
| 1565H    | TLB未命中 → 页表 → 缺页 → 更新TLB → 物理地址（10+100+108+100 = 318） |  |
| 25A5H    | TLB命中 → 物理地址（10+100 = 110）                                      |  |

#### 1565H 逻辑地址转换为物理地址

##### **步骤1：分解逻辑地址**

- **逻辑地址**：`1565H`（十六进制）
- **二进制表示**：`0001 0101 0110 0101`
- **页面大小**：4KB = \(2^{12}\) → 页内偏移占低12位，页号占剩余高位。

| 部分               | 二进制值           | 十六进制值 |
| ------------------ | ------------------ | ---------- |
| 页号（高4位）      | `0001`           | 1          |
| 页内偏移（低12位） | `0101 0110 0101` | 565H       |

---

##### **步骤2：查询页表**

根据题目给出的页表：

| 页号 | 页框号 | 有效位 |
| ---- | ------ | ------ |
| 0    | 101H   | 1      |
| 1    | --     | 0      |
| 2    | 254H   | 1      |

- **页号1的状态**：有效位为0 → **缺页**，需触发缺页中断。

---

##### **步骤3：处理缺页中断**

1. **驻留集限制**：系统为进程分配固定2个物理页框，当前驻留页为0和2。
2. **置换策略**：
   - **LRU算法**：淘汰最近最少使用的页。
   - **局部淘汰策略**：仅淘汰属于该进程的页面（此处为页0或页2）。
3. **淘汰页0**：
   - 假设页0是最近未被访问的页（题目未提供历史访问记录，默认淘汰页0）。
   - 将页0的页框号 `101H` 分配给页1，并更新页表：
     | 页号 | 页框号 | 有效位 |
     | ---- | ------ | ------ |
     | 1    | 101H   | 1      |

---

##### **步骤4：生成物理地址**

- **页框号**：`101H`
- **页框基地址**：`101H << 12 = 101000H`（左移12位对齐）。
- **页内偏移**：`565H`

**物理地址 = 物理块号(101H) 拼接 页内偏移(565H) = `101565H`**

---

### 第5题：FIFO，LRU

> 还是缺页中断，注意 `若该作业的第0页已经装入主存` 。

#### 题目条件

- **字地址序列**：115, 228, 120, 88, 446, 102, 321, 432, 260, 167
- **主存容量**：300字（3个页框，每页100字）
- **初始状态**：第0页已装入内存。

---

#### **关键步骤**

1. **地址到页号的转换**根据页大小（100字），将地址转换为页号：

   | 地址 | 页号 |
   | ---- | ---- |
   | 115  | 1    |
   | 228  | 2    |
   | 120  | 1    |
   | 88   | 0    |
   | 446  | 4    |
   | 102  | 1    |
   | 321  | 3    |
   | 432  | 4    |
   | 260  | 2    |
   | 167  | 1    |

   **页号序列**：`1, 2, 1, 0, 4, 1, 3, 4, 2, 1`
2. **初始内存状态**

   - 初始内存包含页0，剩余两个页框为空。

---

#### 答案

| 算法 | 缺页中断次数 | 缺页率 |
| ---- | ------------ | ------ |
| FIFO | 3            | 3 / 10 |
| LRU  | 4            | 4 / 10 |

---

### 第6题：FIFO与Clock算法解析

> Clock 算法，会考吗？

#### **题目条件**

- **逻辑地址空间与物理地址空间**：均为64KB（按字节编址），即$2^{16}$字节。
- **页大小**：1KB（$2^{10}$字节），页内偏移占10位。
- **页号位数**：逻辑地址总位数16位，页号占6位（16-10=6）。
- **进程需求**：最多需6页，当前分配4个页框（固定分配局部置换策略）。
- **当前页表状态**（时刻260）：

| 页号 | 页框号 | 装入时间 | 访问位 |
| ---- | ------ | -------- | ------ |
| 0    | 7      | 130      | 1      |
| 1    | 4      | 230      | 1      |
| 2    | 2      | 200      | 1      |
| 3    | 9      | 160      | 1      |

- **逻辑地址**：`17CAH`

---

#### **（1）逻辑地址对应的页号**

1. **逻辑地址转换**：
   - `17CAH` = 二进制：`0001 0111 1100 1010`（16位）。
   - **页内偏移**：低10位（`1111 001010`）→ `3CA(H)`。
   - **页号**：高6位（`000101`）→ 十进制为 **5**。

**结论**：
**逻辑地址对应的页号为5**。

---

#### **（2）FIFO置换后的物理地址**

1. **缺页处理**：
   - 页号5不在内存中，触发缺页中断。
   - **FIFO策略**：淘汰最早装入的页。
     - 当前页框分配：页0（装入时间130）、页3（160）、页2（200）、页1（230）。
     - **淘汰页0**，其页框7分配给页5。
2. **物理地址计算**：
   - **页框号7**的二进制表示：`000111`（6位）。
   - **页内偏移**：`3CA(H)`（低10位）。
   - **物理地址**：`000111`（页框号） + `001111001010`（偏移量）
     → 二进制：`0001111111001010` = `1FCA(H)`（十六进制）。

**结论**：
**FIFO置换后的物理地址为1FCA(H)**。

---

#### **（3）Clock置换后的物理地址**

1. **Clock算法流程**：
   - **初始指针指向2号页框**（对应页2，页框号2）。
   - **第一轮扫描**（所有访问位为1）：
     - 页框2（访问位1）→ 置0，指针顺时针移动到页框9。
     - 页框9（访问位1）→ 置0，指针移动到页框7。
     - 页框7（访问位1）→ 置0，指针移动到页框4。
     - 页框4（访问位1）→ 置0，指针回到页框2。
   - **第二轮扫描**：
     - 页框2（访问位0）→ 选中置换。
2. **缺页处理**：
   - **淘汰页2**，其页框2分配给页5。
3. **物理地址计算**：
   - **页框号2**的二进制表示：`000010`（6位）。
   - **页内偏移**：`3CA(H)`（低10位）。
   - **物理地址**：`000010`（页框号） + `001111001010`（偏移量）
     → 二进制：`0000101111001010` = `0BCA(H)`（十六进制）。

**结论**：
**Clock置换后的物理地址为0BCA(H)**。

---

#### **关键总结**

1. **页号计算**：
   - 逻辑地址高6位直接映射页号，低10位为页内偏移。
2. **FIFO策略**：
   - 基于页的装入时间淘汰最早进入的页。
   - 物理地址由页框号 + 偏移量组成。
3. **Clock算法**：
   - 通过循环检查访问位（Reference Bit）选择置换页。
   - 若所有访问位为1，则先置0后重新扫描。
4. **物理地址生成**：
   - 页框号（6位）与页内偏移（10位）组合为16位物理地址，需注意二进制与十六进制的转换。

---

#### **常见问题**

- **页框号如何确定？**页框号由操作系统分配，题目中已明确给出页框号（如页0→7，页1→4等）。
- **Clock算法中指针移动方向？**指针按顺时针方向移动（页框2→9→7→4→2）。
- **访问位（Reference Bit）的作用？**
  标记页面是否被访问过，优先淘汰未被访问的页面（访问位为0）。

---

**最终答案**：

- （1）页号为 **5**。
- （2）FIFO物理地址为 **1FCA(H)**。
- （3）Clock物理地址为 **0BCA(H)**。

---

### 结语 & 更新日志

**预祝大家考试顺利！**

**更新日志**：

- 添加了结语
- 添加了星月夜奶茶鼠
- 优化了一些排版
- 更新了 [hw2](#hw2)
- 移除了 `Him`

![img](../pic/星月夜奶茶鼠.png "没有实际用处的星月夜奶茶鼠，但是可以给人快乐，不是吗")
