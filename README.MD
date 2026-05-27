# **_Jurion Online Judge_**
JurionOnlineJudge以下简称JurionOJ，是一个计算机在线测评系统。兼容强，跨平台，多功能，模块化，轻量化。

访问我们的主站→[这里](http://jurionoj.cc.cd/index.cgi)

相较于其他轻量OJ，JurionOJ具有博客、私信、主页等社交功能，同时支持比赛与判题作业。

对于不想编译的用户，在达到系统默认要求下，可以使用install.sh直接安装。  
兼容主流的Linux发行版，我们推荐使用Debain Wheezy以上的系统。可以使用嵌入式ARM设备，如树莓派。

## _**配置要求**_
内存RAM≥256MB  
CPU最低32位  
网络速率≥100M(CAT.5网线速率)  
## **_软件要求_**  
64或32位系统  
GCC&G++版本≥4.6.3  
Python版本≥2.7    
## **_系统特点与数据结构_**
### 1.数据跨平台
在JurionOJ的data目录中存放着JurionOJ的所有数据，当您备份数据时，可以直接备份data目录的内容。  
### 2.简单的题目上传
用户可以直接粘贴题目描述，测试点可以这样配置。  
```example
1 2||3
8 9||17
```
JurionOJ支持直接在网页上传测试点，测试点数量不要超过50个。
### 3.权限系统
JurionOJ使用灵活的权限系统，分为Su,Admin与User
### 4.社交与通知
JurionOJ内置了私信、博客、个人主页这些社交功能，可以更方便与技术沟通。
### 5.模块化功能
JurionOJ并非将功能硬编码，可以删去对应cgi和注释掉前端代码来禁用功能。
### 6.比赛时间制
JurionOJ支持自由时间制和传统时间制比赛。
### 7.数据结构
在data目录下有这样几个文件和文件夹  
```text
problem_data 题目数据  
testdata 题目测评数据  
sessions 用户登陆一次性凭证  
announcement.txt 公告  
contest_participants.txt 参加比赛用户  
contest_submissions.txt 比赛提交  
contests.txt 比赛列表  
last_id.txt 下一个ID
messages.txt 私信  
posts.txt  博文
problems.txt 题目列表
submissions.txt 提交记录  
users.txt 用户列表
```
### 8.轻量
JurionOJ内存运行时占用80mb，硬盘空间占用38mb，可以在嵌入式设备上完美运行。
## **_安装与使用_**
### 1.准备工具
在操作系统中下载JurionOJ包，使用Git，首先检查您的操作系统中包含Git、GCC、G++以及Python2.7  
`git --version`  
`gcc --version`  
`g++ --version`  
`python --version`  
如果有，继续下一步工作，如果没有则安装  
`sudo apt-get install git`  
`sudo apt-get install gcc`  
`sudo apt-get install g++`  
`sudo apt-get install python`   

### 2.克隆仓库
使用Git克隆仓库到本地  
`git clone https://github.com/Juruoya1/Jurion-Online-Judge.git`  
克隆完成后，进入目录  
`cd Jurion-Online-Judge`  
检查文件是否完整。
### 3.部署
恭喜您，准备工作已经无误可以开始部署工作了。  
如果您的系统符合要求，可以直接运行install.sh，如果不符，可以根据以下步骤手动编译部署。
第一步，编译CERN-HTTPd服务端。  
`cd w3c-httpd-3.0A`  
运行`BUILD.SH`  
安装编译好的CERN-HTTPd
```bash
sudo cp Daemon/linux/httpd /usr/local/sbin/cern-httpd
sudo chmod +x /usr/local/sbin/httpd
```
检查是否成功  
`cern-httpd -v`

编译所有JurionOJ的CGI程序
```bash
cd cgi-src && for f in $(ls *.c | grep -v '\.bak'); do gcc -std=c99 -O2 -Wall -o ../cgi-bin/${f%.c} $f; done && cd ..
```
给予权限
```bash
sudo find ./ -type d -exec chmod 755 {} \;
sudo find ./ -type f -exec chmod 644 {} \;
find ./ -type f \( -name "*.cgi" -o -name "*.pl"  -o -name "*.sh" \) -exec chmod 755 {} \;
```
配置服务器  
如果您已经配置过httpd.conf，那么您的配置文件会被备份在/etc/httpd.conf.bak
```bash
sudo sed -i "s|./cgi-bin|$(pwd)/cgi-bin|" ./httpd.conf
sudo cp /etc/httpd.conf /etc/httpd.conf.bak
sudo cp ./httpd.conf /etc/httpd.conf
```
运行服务器
```bash
cd /mnt/hdd/jurionoj/html
# Start Server
sudo killall cern-httpd 2>/dev/null
sudo /usr/local/bin/cern-httpd -p 8080 &

#Finish
echo "JurionOJ started on port 8080"
echo "Http Link: http://$(hostname -I | awk '{print $1}'):8080"
```

完成，到此结束你已经完整部署了JurionOJ，可以访问输出的ip地址查看index.cgi页面。  

## **_更新日志_**
- V1.0
2026.05.24 发布了第一个版本

## **_联系我们_**
官方邮箱:JurionOJ@outlook.com  
用户QQ群:1084926584  
## **_致谢_**
主要贡献者(首字母A~Z排序):  
IronicFrog60034、Juruoya1  


版权所有 (c) 2026 Jurion。保留所有权利。
SPDX许可证标识符：MIT




# **_Jurion Online Judge_**

JurionOnlineJudge (hereinafter referred to as JurionOJ) is a computer-based online judging system. It is highly compatible, cross-platform, multifunctional, modular, and lightweight.

Visit our main site → [Here](http://jurionoj.cc.cd/index.cgi)

Compared to other lightweight OJs, JurionOJ features social functions such as blogs, private messages, and personal profiles, while also supporting contests and homework judging.

For users who do not wish to compile, you can directly install using `install.sh` provided that the system default requirements are met.

It is compatible with major Linux distributions; we recommend using Debian Wheezy or newer. Embedded ARM devices such as the Raspberry Pi are also supported.

## _**System Requirements**_

- RAM ≥ 256MB
- CPU: minimum 32-bit
- Network speed ≥ 100M (CAT.5 Ethernet speed)

## _**Software Requirements**_

- 64-bit or 32-bit system
- GCC & G++ version ≥ 4.6.3
- Python version ≥ 2.7

## _**System Features & Data Structure**_

### 1. Cross-Platform Data

All data of JurionOJ is stored in the `data` directory. When backing up, you can directly copy the contents of the `data` directory.

### 2. Simple Problem Upload

Users can directly paste problem descriptions. Test points can be configured as follows:

```example
1 2||3
8 9||17
```

JurionOJ supports uploading test points directly via the web page, with a maximum of 50 test points.

### 3. Permission System

JurionOJ uses a flexible permission system, divided into Su, Admin, and User.

### 4. Social Features & Notifications

JurionOJ has built-in social features such as private messages, blogs, and personal profiles, making technical communication more convenient.

### 5. Modular Functions

JurionOJ does not hard-code functions; you can disable features by deleting the corresponding CGI files and commenting out the front-end code.

### 6. Contest Time Modes

JurionOJ supports both free-time mode and traditional time-mode contests.

### 7. Data Structure

The following files and folders are located in the `data` directory:

```text
problem_data
testdata
sessions
announcement.txt
contest_participants.txt
contest_submissions.txt
contests.txt
last_id.txt
messages.txt
posts.txt
problems.txt
submissions.txt
users.txt
```

### 8. Lightweight

JurionOJ uses approximately 80 MB of RAM and 38 MB of disk space at runtime, allowing it to run perfectly on embedded devices.

## _**Installation & Usage**_

### 1. Prepare Tools

Download the JurionOJ package in your operating system using Git. First, check whether your OS has Git, GCC, G++, and Python 2.7:

```bash
git --version
gcc --version
g++ --version
python --version
```

If they are present, proceed to the next step. If not, install them:

```bash
sudo apt-get install git
sudo apt-get install gcc
sudo apt-get install g++
sudo apt-get install python
```

### 2. Clone the Repository

Clone the repository to your local machine using Git:

```bash
git clone https://github.com/Juruoya1/Jurion-Online-Judge.git
```

After cloning, enter the directory:

```bash
cd Jurion-Online-Judge
```

Check that all files are complete.

### 3. Deployment

Congratulations! Your preparation is complete and you can begin deployment.

If your system meets the requirements, you can directly run `install.sh`. If not, follow the steps below to manually compile and deploy.

**Step 1: Compile the CERN-HTTPd server**

```bash
cd w3c-httpd-3.0A
```

Run `BUILD.SH`

Install the compiled CERN-HTTPd:

```bash
sudo cp Daemon/linux/httpd /usr/local/sbin/cern-httpd
sudo chmod +x /usr/local/sbin/httpd
```

Check if successful:

```bash
cern-httpd -v
```

**Compile all JurionOJ CGI programs**

```bash
cd cgi-src && for f in $(ls *.c | grep -v '\.bak'); do gcc -std=c99 -O2 -Wall -o ../cgi-bin/${f%.c} $f; done && cd ..
```

**Set permissions**

```bash
sudo find ./ -type d -exec chmod 755 {} \;
sudo find ./ -type f -exec chmod 644 {} \;
find ./ -type f \( -name "*.cgi" -o -name "*.pl" -o -name "*.sh" \) -exec chmod 755 {} \;
```

**Configure the server**

If you have already configured `httpd.conf`, your configuration file will be backed up at `/etc/httpd.conf.bak`

```bash
sudo sed -i "s|./cgi-bin|$(pwd)/cgi-bin|" ./httpd.conf
sudo cp /etc/httpd.conf /etc/httpd.conf.bak
sudo cp ./httpd.conf /etc/httpd.conf
```

**Run the server**

```bash
cd /mnt/hdd/jurionoj/html
# Start Server
sudo killall cern-httpd 2>/dev/null
sudo /usr/local/bin/cern-httpd -p 8080 &

# Finish
echo "JurionOJ started on port 8080"
echo "Http Link: http://$(hostname -I | awk '{print $1}'):8080"
```

Done! You have successfully deployed JurionOJ. Visit the output IP address to see the `index.cgi` page.

## _**Changelog**_

- **V1.0** – 2026.05.24 – First version released

## _**Contact Us**_

- Official email: JurionOJ@outlook.com
- User QQ Group: 1084926584

## _**Acknowledgments**_

Main contributors (sorted by first letter A~Z):  
IronicFrog60034, Juruoya1

---

**Copyright (c) 2026 Jurion. All rights reserved.**  
SPDX License Identifier: MIT
