# Vivado 创建 Acoral工程简易流程 

## 一、创建硬件工程

### 1.1 使用工程向导创建项目

启动`vivado`, 点击Quick Start栏的 Create Project, 然后在弹出的工程向导界面点击 Next, 如图

![image-20240303233119121](attachments/image-20240303233119121.png)



输入项目名称和路径并点击 next

![image-20240303233408522](attachments/image-20240303233408522.png)

继续 next

![image-20240303233444791](attachments/image-20240303233444791.png)

注意：根据芯片不同选择不同型号
zynq7020: 选择 xc7z020clg400-2

![image-20240303233602528](attachments/image-20240303233602528.png)
确认界面

![image-20240303233618593](attachments/image-20240303233618593.png)

zynq7045选择：
xc7z045ffg900-2
![[acoral_zynq7020_vivado项目创建--20250221.png]]

确认界面：
![[acoral_zynq7020_vivado项目创建--20250221-2.png]]

### 1.2 创建ip

点击左侧 IP INTEGRATOR 中的 Create Block Design, 设置设计的名称 system(都行), 点击OK

![image-20240303233848339](attachments/image-20240303233848339.png)

点击 Diagram 中的加号并搜索双击选中ZYNQ7,

![image-20240303233938935](attachments/image-20240303233938935.png)

双击出现的模块进行配置

![image-20240303234125356](attachments/image-20240303234125356.png)

### 1.2 配置ip

#### 1.2.1 配置外设

1）配置uart

![image-20240303234324770](attachments/image-20240303234324770.png)

2）配置SD(重要!! 没有这个fatfs会出错!!!)

![image-20240304000203369](attachments/image-20240304000203369.png)

3）添加以太网口

![image-20241227132222699](attachments/image-20241227132222699.png)

4）配置MIO参数（并且SD还需要设置电压和检测用的CD信号，否则用不了

![image-20240710190751157](attachments/image-20240710190751157.png)

![image-20241227134901381](attachments/image-20241227134901381.png)

![image-20241227135004974](attachments/image-20241227135004974.png)

5）添加MIO

![image-20240305103328234](attachments/image-20240305103328234.png)

#### 1.2.2 配置DDR
7020配置该型号：

![image-20240303234435277](attachments/image-20240303234435277.png)

7045配置该型号：
![[acoral_zynq7020_vivado项目创建--20250221-1.png]]

#### 1.2.3 PL（FPGA）部分配置

取消pl时钟

![image-20240303234539001](attachments/image-20240303234539001.png)

关闭pl和ps交流接口

![image-20240303234644362](attachments/image-20240303234644362.png)

![image-20240303234706218](attachments/image-20240303234706218.png)

![image-20240303234745501](attachments/image-20240303234745501.png)

点击OK

![image-20240303234810429](attachments/image-20240303234810429.png)

### 1.3 导出硬件

#### 1.3.1 生成硬件代码

点击 Run Block Automation

![image-20240303234831205](attachments/image-20240303234831205.png)

点击OK

![image-20240303234851892](attachments/image-20240303234851892.png)

点击检查

![image-20240303234951693](attachments/image-20240303234951693.png)

接下来生成顶层模块

![image-20240303235115905](attachments/image-20240303235115905.png)

![image-20240303235201586](attachments/image-20240303235201586.png)

![image-20240303235240478](attachments/image-20240303235240478.png)

生成HDL Wrapper

![image-20240303235331484](attachments/image-20240303235331484.png)

![image-20240303235413882](attachments/image-20240303235413882.png)

#### 1.3.2 导出硬件

![image-20240303235452072](attachments/image-20240303235452072.png)

![image-20240303235518426](attachments/image-20240303235518426.png)

#### 1.3.3 打开SDK

![image-20240303235546932](attachments/image-20240303235546932.png)

得到

![image-20240303235622137](attachments/image-20240303235622137.png)

## 二、创建软件项目

### 2.1 创建新项目

关闭自动构建

![image-20240305115335455](attachments/image-20240305115335455.png)

新建项目

![image-20240303235825045](attachments/image-20240303235825045.png)

![image-20240305103810575](attachments/image-20240305103810575.png)

![image-20240305103831194](attachments/image-20240305103831194.png)

### 2.2 配置BSP

添加ffs

![image-20240304001628076](attachments/image-20240304001628076.png)

![image-20240304001709512](attachments/image-20240304001709512.png)

### 2.3 导入源代码

#### 2.3.1 拷贝代码目录

删除sdk/acoral_II/下的src, 复制这六个目录，并且右键项目刷新

![image-20240304000831041](attachments/image-20240304000831041.png)

![image-20241227132714055](attachments/image-20241227132714055.png)

![image-20240305115229315](attachments/image-20240305115229315.png)

#### 2.3.2 替换两个汇编文件内容

用vector.txt和boot.txt替换位于xx\project\project.sdk\acoral_II_bsp\ps7_cortexa9_0\libsrc\standalone_v6_8\src的两个文件内容

![image-20241227133019777](attachments/image-20241227133019777.png)

#### 2.3.3 添加路径

右键acoral_II, 选择properties

![image-20240304001300558](attachments/image-20240304001300558.png)

![image-20240304001414959](attachments/image-20240304001414959.png)

![[acoral_zynq7020_vivado项目创建--20250328.png]]
#### 2.3.4 添加库

![image-20240304002042010](attachments/image-20240304002042010.png)

#### 2.3.5 更改编码为UTF8

![image-20240305104642466](attachments/image-20240305104642466.png)

![image-20240305104813161](attachments/image-20240305104813161.png)

### 2.4 调试

#### 2.4.1 首先进行一次调试

第一次调试右键项目

![image-20240305115901346](attachments/image-20240305115901346.png)

#### 2.4.2 修改调试配置

这里不管是否成功都要修改调试配置

![image-20241227133551864](attachments/image-20241227133551864.png)

![image-20240304123632029](attachments/image-20240304123632029.png)

![image-20240305120034658](attachments/image-20240305120034658.png)

#### 2.4.3 正常进行调试

![image-20240304123845999](attachments/image-20240304123845999.png)



























