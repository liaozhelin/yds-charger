# 四路桌面智能充电站

> 这是一个实用型智能多路桌面充电器项目
>
> 视频介绍：[【开源】如何自制一个全能桌面充电站？](https://www.bilibili.com/video/BV1X94y1U7av/)
>
> 如果觉得视频还可以的话，留下一键三连哦 ( ˊ•̥▵•)੭₎₎
>
> 本项目欢迎复刻，禁止商用！！
>
> 整体成本（纯硬件）在50元左右，目前没有贵的离谱芯片，选用的芯片基本都是国产使用量较高的
>
> 有任何问题请在`issues`中提交哈，包括软件功能建议，其他平台的消息不一定看得到👀

![5E3306F0C8ED8A1241D5FD975B7F77DC](https://raw.githubusercontent.com/liaozhelin/picgo/master/5E3306F0C8ED8A1241D5FD975B7F77DC.png)

**仓库目录说明**

1. `Hardware`：【上层板/下层板/前面板/后面板 】gerber文件/原理图源文件/原理图PDF文件/坐标文件/BOM文件
2. `Firmware`：【 ESP32C3FN4 的固件以及工具 】固件/驱动/下载烧录软件
3. `Software`： 【 软件工程源码 】esp-idf-4.4 版本开发环境
4. `Structure`： 【 整机结构文件 】包含电路板及外壳装配文件，软件版本Soliworks2022
5. `Documents`： 【 项目所用到的文档手册等 】部分手册，有些手册是厂家不让流传的没有上传

**资料说明 （更新日期22-5-30）**

- 已添加gerber文件，包含上层版，下层板，前面板，后面板，可直接打样（上层板若审核不通过备注断板不补即可）
- 已添加原理图源文件
- 已添加 README 文件 
- 已添加烧录软件,bootloader固件以及app固件
- 已添加结构设计文件夹
- 已添加手册文件夹
- 已添加 esp32c3 usb cdc/jtag驱动文件
- 已添加源码工程文件夹，希望大佬们可以帮忙改进，代码里有什么错误以及疑问都可以提交issue交流，当然能帮我完善功能就更好了😊
- 已提交新版上层板 (Gerber_topBoard-Fix.zip) ，修改掉不好生产的边框处后两层板实在没办法走通，改成四层板了，打样一样免费，就是颜色可能也得绿色了，可以先试试原版两层板，有概率通过，不通过再换成新版上层板。👀
- 已添加上下层板的动态BOM表，说明文件，可以方便焊接选料，若打样嘉立创不通过可以考虑捷配，新用户也可以免费打板审核也比较松。
- 已添加器件采购指南，只做样例，若链接失效自行找类似器件替换。
- 已添加不同丝印版本的文件，在每个板子文件夹中的others文件夹中，每一块板子都提供了3种不同的丝印版本，降低打样时候判定拆单的概率（实在不行就换其他打板公司或者换不同时间打样，基本上只要是机审就可以通过）

## 一、项目介绍：

### 1.1 硬件选型：

> 本项目的硬件购买都来自于淘宝，虽然有可能有假货，但是我没有翻车，大家可以酌情选择。

- 项目采用乐鑫**ESP32C3FN4**芯片作为控制MCU，具备Wifi和Bluetooth，价格便宜功能强大，可供二次开发增加许多其他功能(内置4M的FLASH)。
- C口功率级采用了智融**SW3526**的功率芯片（目前价格4.5元一片😍），该芯片支持多种快充协议，芯片内部集成功率管，减少PCB面积，最大功率为单通道65W，本项目中设计了两组通道。
- A口功率级采用了英集芯的**IP6525T**，价格低廉（目前价格1元一片😍），最大功率为单通道18W，本项目中设计了两路, 设计在机身后部, 主要是给桌面上的固定设备供电, 比如台灯, 小服务器等, 可通过MOS管远程控制开关
- 交互界面显示屏采用了**0.96**寸OLED屏（尺寸刚好,至于烧屏问题后面会通过屏保解决）。
- 输入部分设计了保护电路，具备防反接（方便车充应用）功能，反接系统不会工作。具备过流保护电路（10A），过流会熔断保险丝。具备防打火缓启动电路，可以防止上电火花。
- 板载四路RGB指示灯以及面板，可以实现当前系统状态的快速指示。
- 加速度传感器为**LIS3DH**，可以实现震动控制或者体感操作等功能。

![3](https://raw.githubusercontent.com/liaozhelin/picgo/master/3.jpg)

### 1.2 项目参数

输入电压：10V-32V直流 (推荐使用换下来的笔记本充电器,咸鱼便宜量大,当然更推荐24V5A台达电源,可以最大程度发挥功率)

输入功率：＞80W

快充参数如下：

- C口输出协议：PPS/PD3.0/PD2.0/QC4+/QC4/QC3.0/QC2.0/AFC/FCP/SCP/PE2.0/PE1.1/SFCP/BC2.1/苹果/三星
- C口输出电压：0.3-22V
- C口输出电流：0-3.3A
- A口输出协议：DCP/QC2.0/QC3.0/FCP/AFC
- A口输出电压：0.3-12V
- A口输出电流：0-3A

PCB参数：47*80(mm) 厚度1.6mm 四层板

整体尺寸（含外壳）：50X*20X*80(mm)

## 二、制作教程

### 2.1 制作注意事项

> 本产品焊接完成，测试无短路虚焊后，可通过下载口USB连接电脑，烧写程序后即可使用（ESP32C3自带内部USB和JTAG,十分方便）,如果不识别可能需要自行下载C3驱动安装即可

- 装机时请注意功率级部分的散热，推荐在功率级部分背面垫导热片增强散热能力。
- 焊接推荐采用钢网焊接，本项目中器件均为单面布置，可以快速实现低成本回流焊。
- 初次上电时，推荐使用5V电源，测量RY8411输出正常3.3V后才可以升高电压进行后面的测试。
- C1口为板载，C2口通过排针在上层板中引出。

上层板和下层板装配采用常规排针排母，但是排针需要**特殊处理**下，去掉塑料部分直接焊接，不然槽间高度不够：

| 第一步                                                       | 第二步                                                       | 第三步                                                       | 第四步                                                       | 第五步                                                       |
| ------------------------------------------------------------ | ------------------------------------------------------------ | ------------------------------------------------------------ | ------------------------------------------------------------ | ------------------------------------------------------------ |
| <img src="https://raw.githubusercontent.com/liaozhelin/picgo/master/picpath/image-20220613213737766.png" alt="image-20220613213737766" style="zoom:50%;" /> | <img src="https://raw.githubusercontent.com/liaozhelin/picgo/master/picpath/image-20220613213846373.png" alt="image-20220613213846373" style="zoom:50%;" /> | <img src="https://raw.githubusercontent.com/liaozhelin/picgo/master/picpath/image-20220613213913880.png" alt="image-20220613213913880" style="zoom:50%;" /> | <img src="https://raw.githubusercontent.com/liaozhelin/picgo/master/picpath/image-20220613213958565.png" alt="image-20220613213958565" style="zoom:50%;" /> | <img src="https://raw.githubusercontent.com/liaozhelin/picgo/master/picpath/image-20220613214043013.png" alt="image-20220613214043013" style="zoom:50%;" /> |
| 将排针插进上层板，顶在桌面上，往下按                         | 取出后可以得到如上排针，按照这个样式制作三组                 | 用水口钳去掉塑料部分，得到针脚                               | 将排针插进下层板焊接好的排母中                               | 将上层板子和下层板紧贴，排针插进洞中，然后焊接，这个距离刚刚好是两槽间距离 |

上层板的上面焊接完后，可以在上层铝壳对应位置贴上黑胶布绝缘（不贴也可以，实际上还有一定距离，保险起见）

### 2.2 固件烧录教程

> 通过Fireware/tools文件夹中的 flash_download_tool_3.9.2 工具下载
>
> 本项目暂时不提供PCB源文件，源码已经全部开源，有需要二次开发的朋友可以自行修改

​	本项目不需要烧录器/下载器，只需要你有一个USB线，但是需要自己焊接一个降压电路将USB口的5V转到3.3V（或者不从排针3.3V处供电，直接通过DC口供电）,**一定不要把5V接到板子上的3.3V处，必烧ESP32**，我是使用了一个洞洞板+AMS1117焊接的简单下载器：

| <img src="https://raw.githubusercontent.com/liaozhelin/picgo/master/image-20220528000844883.png" alt="image-20220528000844883" style="zoom: 33%;" /> | <img src="https://raw.githubusercontent.com/liaozhelin/picgo/master/picpath/image-20220613145939279.png" alt="image-20220613145939279" style="zoom:50%;" /> |
| ------------------------------------------------------------ | ------------------------------------------------------------ |

1. 合并固件位置在 `Fireware/bin/target.bin` ，独立固件位置在 `Software/yds_charger` 中，推荐不打算自己后续开发的使用合并bin下载方式。

2.  软件打开后将设备连接到电脑，如图片`1`设置好下载模式后，点击OK。

3. 【合并bin下载方式】按照下面的图片`2 `设置好软件，点击下载应该就可以下载固件了，一共需要烧录一个固件即可，地址设置0

4. 【独立bin下载方式】按照下面的图片`3 `设置好软件，点击下载应该就可以下载固件了,一共需要烧录四个固件，注意地址设置

   > 如果出现连接错误可以将上板后面的拨动开关向左拨动不松，然后再上电，就会进Boot模式）烧录完成，断电后重新上电，应该就可以正常使用了。

​	烧录完成后，重新上电复位，就可以使用了。

|                              1                               |                              2                               |                              3                               |
| :----------------------------------------------------------: | :----------------------------------------------------------: | :----------------------------------------------------------: |
| <img src="https://raw.githubusercontent.com/liaozhelin/picgo/master/image-20220528000432670.png" alt="image-20220528000432670" style="zoom: 67%;" /> | <img src="https://raw.githubusercontent.com/liaozhelin/picgo/master/picpath/image-20220613145429212.png" alt="image-20220613145429212" style="zoom:33%;" /> | <img src="https://raw.githubusercontent.com/liaozhelin/picgo/master/picpath/image-20220613144244227.png" alt="image-20220613144244227" style="zoom: 33%;" /> |

​	ESP32 采用了 APP+OTA1+OTA2 的乒乓更新分区（这样就算更新到一半断电断网也不会变砖），可以实现远程更新，首次烧录完后续盖上盖子后，之后就可以使用无线更新了。

## 三、IDF开发

### 3.1 开发环境搭建

​	IDF开发环境我是在 Windows 系统中使用 `VScode+idf拓展插件` 在Python虚拟环境中实现开发的，实际上也可以在Linux中直接搭建环境开发，或者使用`WSL+VScode远程插件` 也是可以的，可以根据自己的需要进行选择，详细的搭建步骤可以见[官网搭建流程](https://docs.espressif.com/projects/esp-idf/zh_CN/release-v4.1/get-started/index.html#get-started-get-prerequisites)：

​	下面以我的`VScode+idf拓展插件` 搭建环境为例：

1. 安装Vscode，这个直接下载安装即可，然后在左侧的应用市场中，搜索关键词 Espressif IDF，安装：

   <img src="https://raw.githubusercontent.com/liaozhelin/picgo/master/picpath/image-20220613131702897.png" alt="image-20220613131702897" style="zoom:50%;" />

2. 插件安装完成后，按`F1`,在命令中找到`Configure ESP-IDF extension`选项，点击：

   <img src="https://raw.githubusercontent.com/liaozhelin/picgo/master/picpath/image-20220613131821004.png" alt="image-20220613131821004" style="zoom: 67%;" />

3. 选择ADVANCED模式，可以详细配置一些选项：

   ![image-20220613131919778](https://raw.githubusercontent.com/liaozhelin/picgo/master/picpath/image-20220613131919778.png)

4. 依次设置软件源，IDF版本，IDF路径，IDF工具路径。软件源如果你科学就可以直接选择Github，没有科学可以选择Espressif的国内源，IDF版本这里选择`release/4.4`,其他两个可以根据自己进行设置（路径中不要出现中文或者空格），推荐使用默认路径，然后点击 `Install` 进行安装：

   <img src="https://raw.githubusercontent.com/liaozhelin/picgo/master/picpath/image-20220613132213211.png" style="zoom:50%;" />

5. 等待ESP-IDF下载安装完成，IDF整体体积不小，具体看网速：

   <img src="https://raw.githubusercontent.com/liaozhelin/picgo/master/picpath/image-20220613132432305.png" alt="image-20220613132432305" style="zoom:50%;" />

6. 接下来安装ESP-IDF工具，其中包含虚拟环境以及虚拟环境中用到的一些工具链，选择 `Download ESP-IDF Tools`选项，点击 `Download Tools`：

   ![image-20220613133019914](https://raw.githubusercontent.com/liaozhelin/picgo/master/picpath/image-20220613133019914.png)

7. 等待工具安装完成：

   ![image-20220613133149569](https://raw.githubusercontent.com/liaozhelin/picgo/master/picpath/image-20220613133149569.png)

8. 若出现如下页面，则安装完成，如果上述中有步骤报错，一般就是网络问题：

   <img src="https://raw.githubusercontent.com/liaozhelin/picgo/master/picpath/image-20220613133812483.png" alt="image-20220613133812483" style="zoom: 67%;" />

### 3.2导入工程开发

​	本项目的工程在 `Software/yds_charger` 下，将项目克隆到本地，主要修改项目中的.vscode文件夹，这个文件夹中的文件控制着IDF插件的功能以及Vscode本身的代码高亮提示跳转等。

​	先验证安装环境是否可以正常使用，按`F1`,在命令中找到`New Project`,点击：

<img src="https://raw.githubusercontent.com/liaozhelin/picgo/master/picpath/image-20220613133845390.png" alt="image-20220613133845390" style="zoom:50%;" />

​	根据自己喜好修改工程名，路径，板子，串口（没接上电脑就不用选），最后一个选项可以不管，点击 `Chose Template`：

![image-20220613134033428](https://raw.githubusercontent.com/liaozhelin/picgo/master/picpath/image-20220613134033428.png)

​	下拉框中选择 `ESP-IDF` ,这里可以选择一个最简单的项目测试下，`sample_project`，选择好后点击蓝色按钮生成工程，并信任文件夹中的作者并启用所有功能:

<img src="https://raw.githubusercontent.com/liaozhelin/picgo/master/picpath/image-20220613134236762.png" alt="image-20220613134236762" style="zoom:50%;" />

​	点击工程下面框中的编译按钮，等待编译完成，若能成功生成bin文件则环境搭建成功（编译如果很慢的话，关闭Windows的病毒防护，可以极大提高编译速度）：

<img src="https://raw.githubusercontent.com/liaozhelin/picgo/master/picpath/image-20220613134532026.png" alt="image-20220613134532026" style="zoom:67%;" />

<img src="https://raw.githubusercontent.com/liaozhelin/picgo/master/picpath/image-20220613135352420.png" alt="image-20220613135352420" style="zoom:50%;" />

​	验证完编译环境后，可以将示例工程中.vscode中文件复制到克隆下的工程中，或者使用 `F1` 中的添加.vscode文件命令也可以，应该包含下面四个文件：

![image-20220613142504698](https://raw.githubusercontent.com/liaozhelin/picgo/master/picpath/image-20220613142504698.png)

​	后面就可以愉快地开发了，烧录下载也一样是用下面的按钮，这里就不赘述了：

<img src="https://raw.githubusercontent.com/liaozhelin/picgo/master/picpath/image-20220613142901062.png" alt="image-20220613142901062" style="zoom:67%;" />



**功能开发（更新日期22-5-27）：**

✅ 已经开发完成   🚩正在开发中   ❌还未开发或放弃开发

|          设计功能          | 状态 |               说明               |
| :------------------------: | :--: | :------------------------------: |
|    远程网络升级（OTA）     |  ✅   |         带固件版本号校验         |
|          WIFI配网          |  ✅   |       通过ESPTouch软件配网       |
|         主界面显示         |  ✅   |        待机休眠屏保待完善        |
|        快充协议解析        |  ✅   |              已实现              |
|         RGB指示灯          |  ✅   |      已实现，控制逻辑待完善      |
|      各口电压电流监测      |  ✅   |              已实现              |
|        过流反接保护        |  ✅   |          以实现，已校验          |
|   远程控制充电器四口开关   |  🚩   |      硬件已验证，还未写逻辑      |
|     加速度计(重力感应)     |  🚩   |      程序已驱动，还未写功能      |
|        C口功率限制         |  🚩   |     程序已实现，界面还未增加     |
|    OTA自动检测版本升级     |  🚩   | 版本验证已完成，json解析还在修改 |
| 电脑CPU占用，内存占用显示  |  ❌   |             还未开发             |
|          天气显示          |  ❌   |             还未开发             |
|         粉丝数显示         |  ❌   |             还未开发             |
|        重力旋转屏幕        |  ❌   |             还未开发             |
| 蓝牙广播检测手环自动开关机 |  ❌   |             还未开发             |
|                            |      |                                  |
|                            |      |                                  |
|                            |      |                                  |

......

未完待续

......

> 本项目禁止商用，仅供同好们复刻使用，在制作项目时候，请确保硬件无故障再连接设备使用（我已经使用了两个月，目前没有出过问题，手头设备不是很多，所以测试可能不是很完善，不能保证系统不存在隐藏的问题。希望大家能发现问题一起改进），对于造成高价值设备损坏以及其他连带损失的的，本人不承担任何责任哈。
>
> 海拔高度：＜ 2000米
>
> 环境湿度：≤ 75% RH
>
> 工作环境：0℃ - 50摄氏度

纪念一下开发过程中的炮灰小弟们，第一代和第三代（总共迭代了5版本，其中有2代搬板拆了，剩下这两个完好的）：

<img src="https://raw.githubusercontent.com/liaozhelin/picgo/master/image-20220528181253005.png" alt="image-20220528181253005" style="zoom:50%;" />

