# aCoral

支持混合实时调度的嵌入式实时操作系统



<!-- PROJECT SHIELDS -->

[![Contributors][contributors-shield]][contributors-url]
[![Forks][forks-shield]][forks-url]
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]
[![Apache License][license-shield]][license-url]

<!-- PROJECT LOGO -->
<br />

<p align="center">
  <a href="https://github.com/EIC-UESTC/aCoral/">
    <img src="docs/images/acoral_870x270.png" alt="Logo" width="210" height="">
  </a>

  <h3 align="center">珊瑚aCoral实时操作系统</h3>
  <p align="center">
    一个嵌入式实时操作系统
    <br />
    <a href="https://github.com/EIC-UESTC/aCoral/tree/main/docs"><strong>探索本项目的文档 »</strong></a>
    <br />
    <br />
    <a href="https://github.com/EIC-UESTC/aCoral">查看Demo</a>
    ·
    <a href="https://github.com/EIC-UESTC/aCoral/issues">报告Bug</a>
    ·
    <a href="https://github.com/EIC-UESTC/aCoral/issues">提出新特性</a>
  </p>
</p>

## 目录

- [上手指南](#上手指南)
  - [开发前的配置要求](#开发前的配置要求)
  - [安装步骤](#安装步骤)
- [文件目录说明](#文件目录说明)
- [开发的架构](#开发的架构)
- [部署](#部署)
- [使用到的框架](#使用到的框架)
- [贡献者](#贡献者)
  - [如何参与开源项目](#如何参与开源项目)
- [版本控制](#版本控制)
- [作者](#作者)
- [鸣谢](#鸣谢)

### 上手指南

查看`文档`了解如何创建项目


###### 开发前的配置要求

`xilinx7020/7045`平台
`windows` +` vivado`开发环境

###### **安装步骤**

1. 克隆仓库到本地
```sh
git clone https://github.com/EIC-UESTC/aCoral.git
```
2. 按照文档中创建vivado项目
3. 构建调试

### 文件目录说明


```
aCoral
  ├─bsp         平台相关代码及应用
  ├─components  系统组件
  ├─docs        文档
  ├─include     包含目录
  ├─kernel      内核代码
  ├─libcpu      架构相关
  ├─modify      创建项目时要修改的文件
  └─src         链接脚本所在目录
```

### 开发的架构 

暂无

### 部署

暂无

### 使用到的框架

暂无

### 贡献者

请阅读**CONTRIBUTING.md** 查阅为该项目做出贡献的开发者。

#### 如何参与开源项目

贡献使开源社区成为一个学习、激励和创造的绝佳场所。你所作的任何贡献都是**非常感谢**的。


1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request



### 版本控制

该项目使用Git进行版本管理。您可以在repository参看当前可用版本。

### 作者



### 版权说明

该项目签署了 Apache 授权许可，详情请参阅 [LICENSE](https://github.com/EIC-UESTC/aCoral/blob/master/LICENSE)

### 鸣谢

- [Img Shields](https://shields.io)
- [GitHub Pages](https://pages.github.com)

<!-- links -->
[your-project-path]:EIC-UESTC/aCoral
[contributors-shield]: https://img.shields.io/github/contributors/EIC-UESTC/aCoral.svg?style=flat-square
[contributors-url]: https://github.com/EIC-UESTC/aCoral/graphs/contributors
[forks-shield]: https://img.shields.io/github/forks/EIC-UESTC/aCoral.svg?style=flat-square
[forks-url]: https://github.com/EIC-UESTC/aCoral/network/members
[stars-shield]: https://img.shields.io/github/stars/EIC-UESTC/aCoral.svg?style=flat-square
[stars-url]: https://github.com/EIC-UESTC/aCoral/stargazers
[issues-shield]: https://img.shields.io/github/issues/EIC-UESTC/aCoral.svg?style=flat-square
[issues-url]: https://img.shields.io/github/issues/EIC-UESTC/aCoral.svg
[license-shield]: https://img.shields.io/github/license/EIC-UESTC/aCoral.svg?style=flat-square
[license-url]: https://github.com/EIC-UESTC/aCoral/blob/master/LICENSE
