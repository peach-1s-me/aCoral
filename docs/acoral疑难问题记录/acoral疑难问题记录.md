```
## 2025-xx-xx
### 1. xx问题(已解决)
> 记录人: 
#### 1）现象

#### 2）代码

#### 3）分析(与解决)

```

## 2025-04-07
### 1. 核间迁移开销测试中开销分布很分散(已解决)
> 记录人: 文佳源、饶洪江
#### 1） 现象
核间迁移开销测试时开销分布超级分散，数据存在多个波峰
数据如下：
```
15      255
16      13629
17      10499
18      64
19      1
20      66
21      866
22      3985
23      8777
24      7205
25      2735
26      437
27      22
40      52
41      4054
42      15606
43      4990
44      47
47      1
48      1
49      24
50      86
51      145
52      100
53      30
54      9
59      4
60      72
61      199
62      126
63      19
68      7
69      100
70      307
71      144
72      38
73      9
103     2
104     1
105     18
106     24
107     53
108     122
109     180
110     205
111     131
112     74
113     31
114     12
115     2
130     3
131     7
132     14
133     42
134     97
135     125
136     90
137     55
138     41
139     94
140     400
141     1161
142     2468
143     3775
144     4155
145     3383
146     2149
147     1000
148     354
149     103
150     21
151     15
152     47
153     126
154     270
155     356
156     286
157     134
158     43
159     8
160     2
163     14
164     50
165     189
166     435
167     769
168     851
169     694
170     413
171     161
172     24
173     9
193     1

```
#### 2） 代码
线程1：优先级为2，执行空循环
线程2：优先级为1，执行迁移线程1操作
```c
void empty_entry(void *args)
{
	acoral_print("Empty thread start running\r\n");
	while(1);
}

void move_thread(void *args)
{
    acoral_print("measure move enter\r\n");

    acoral_thread_t * ep_task = (acoral_thread_t *)acoral_get_res_by_id(ep_id);
    while(1)
    {
        if (move_buffer.record_num == MOVE_TIMES)
        {
            measure_done("move thread", &move_buffer);
            break;
        }

        acoral_enter_critical();
        if(ep_task->cpu == 0)
        {
            cal_time_start();
        	acoral_moveto_thread(ep_task, 1);
        }
        else
        {
            cal_time_start();
        	acoral_moveto_thread(ep_task, 0);
        }
        acoral_exit_critical();
        double during = cal_time_end();
        if (during > 0)
        {
            push_during(&move_buffer, during);
        }
    }
}
```
#### 3）分析(与解决)
饶洪江怀疑是用来移动的线程进行迁移的频率太快，发送的核间中断可能响应不过来。
因此尝试在迁移操作并存储开销数据后增加delay：
```c
if(ep_task->cpu == 0)
{
    cal_time_start();
	acoral_moveto_thread(ep_task, 1);
}
else
{
    cal_time_start();
	acoral_moveto_thread(ep_task, 0);
}
acoral_exit_critical();
double during = cal_time_end();
if (during > 0)
{
    push_during(&move_buffer, during);
}
acoral_delay_ms(5);
```
然后数据正常:
```
== test move thread ==
[measure move] create empty thread, which is used to be moved
[measure] create measure threads
Empty thread start running
measure move enter
measure move thread done
record 100000 data, time unit is us
15      1420
16      36444
17      12135
18      1
40      63
41      8215
42      34149
43      7562
44      11
```
有两个波峰的原因：核间迁移函数中有两个开销不同的分支路径，因此是合理现象

### 2. 周期线程调度开销测试中波峰奇怪变化(已解决)
> 记录人: 饶洪江、文佳源
#### 1）现象
(创建的周期线程周期为2ms，也就是2个tick)
测试周期线程调度开销时发现开销的数据分布和创建的周期线程数相关，并且规律不太明显
只有一个线程的时候：
```
== test schedule ==
[measure] create schedule measure threads
measure period schedule
1 period thread created
wait measure schedule enter
measure schedule done
record 10000 data, time unit is us
1       4
2       4995
3       1
26      700
27      4293
28      7
30      1
```
两个线程：
```
== test schedule ==
[measure] create schedule measure threads
measure period schedule
2 period thread created
wait measure schedule enter
measure schedule done
record 10000 data, time unit is us
1       3
2       4996
3       1
59      237
60      3493
61      1268
62      2
66      1
```
三个线程:
```
== test schedule ==
[measure] create schedule measure threads
measure period schedule
3 period thread created
wait measure schedule enter
measure schedule done
record 10000 data, time unit is us
2       5000
93      43
94      1832
95      2414
96      625
97      80
98      4
99      1
100     1
```
四个线程:
```
== test schedule ==
[measure] create schedule measure threads
measure period schedule
4 period thread created
wait measure schedule enter
measure schedule done
record 10000 data, time unit is us
1       35
2       4964
3       1
148     1
149     57
150     450
151     1176
152     1510
153     1156
154     523
155     112
156     16
157     1
```
五个线程:
```
== test schedule ==
[measure] create schedule measure threads
measure period schedule
5 period thread created
wait measure schedule enter
measure schedule done
record 10000 data, time unit is us
2       1
65      291
66      2990
67      1711
68      7
70      1
97      7
98      840
99      3218
100     925
101     12
103     1
```
六个线程：
```
== test schedule ==
[measure] create schedule measure threads
measure period schedule
6 period thread created
wait measure schedule enter
measure schedule done
record 10000 data, time unit is us
2       1
99      3
100     495
101     5375
102     3921
103     205
104     1
106     1
107     1
```
#### 2）代码
```c
/* 等待测试结束 */
void wait_measure_schedule(void *args)
{
    acoral_print("wait measure schedule enter\r\n");
    while(1)
    {
        if (sched_buffer.record_num >= SCHEDULE_TIMES)
        {
            measure_done("schedule", &sched_buffer);
            break;
        }
    }
}

/* 创建多个周期线程，执行空操作，以测试调度 */
void empty_period_entry(void *args)
{

}
```

#### 3）分析(与解决)
观察数据可知创建四个以上周期线程会导致出现两个30和`30*k`的波峰，其中个位数us的开销表示当前没有周期线程需要装载（包括重新装堆栈等），并且由于周期为2，因此不同周期线程之间的相位差值只可能是0或1ms，因此会出现两个波峰（若出现30的波峰则是因为每个tick都会有周期线程被重新装载）
考虑是创建多个周期线程过程中前面创建的周期线程被到来的时钟中断中周期线程处理函数将线程的下个到来时间（周期线程时刻队列中的值）-1，因此不同周期线程会出现相位差。故如果想要创建的多个周期线程等同于同一时刻创建，需要将这一组创建操作放在临界区中进行：
```c
acoral_enter_critical();
for(i=0; i<PERIOD_THREAD_NUM; i++)
{
    epp_id[i]=acoral_create_thread(
                empty_period_entry,
                MEASURE_TASK_STACK_SIZE,
                NULL,
                "empty_period",
                NULL,
                ACORAL_SCHED_POLICY_PERIOD,
                &period_priv_data,
                NULL,
                NULL
            );
    if(epp_id[i]==-1)
    {
        while(1);
    }
}
acoral_print("%d period thread created\r\n", PERIOD_THREAD_NUM);
acoral_exit_critical();
```
修改后，创建一个周期线程时：
```
== test schedule ==
[measure] create schedule measure threads
measure period schedule
1 period thread created
wait measure schedule enter
measure schedule done
record 10000 data, time unit is us
1       816
2       4184
25      1
26      1431
27      3566
28      1
29      1
31      1
```
两个线程:
```
== test schedule ==
[measure] create schedule measure threads
measure period schedule
2 period thread created
wait measure schedule enter
measure schedule done
record 10000 data, time unit is us
1       3
2       4997
59      20
60      1542
61      3129
62      307
63      2
67      1
```
三个:
```
== test schedule ==
[measure] create schedule measure threads
measure period schedule
3 period thread created
wait measure schedule enter
measure schedule done
record 10000 data, time unit is us
1       8
2       4992
93      44
94      1789
95      2563
96      560
97      41
98      3
100     1
```
四个:
```
== test schedule ==
[measure] create schedule measure threads
measure period schedule
4 period thread created
wait measure schedule enter
measure schedule done
record 10000 data, time unit is us
1       870
2       4130
148     1
149     17
150     241
151     895
152     1411
153     1431
154     775
155     209
156     20
157     1
159     1
```