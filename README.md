# computer-networks
the experiment of computer networks

计算机网络实验一：

基本任务：实现回退N协议和选择重传协议。

数据链路层的协议，停等协议，回退N协议，选择重传协议。

协议都已经实现√，若要进行验证，只需要将datalink.c里的内容替代为某个协议即可。解决方案里并没有将datalink_stopwait.c , 
datalink_go_back_n.c ,datalink_selective_repeat.c 文件引用，只引用了datalink.c，所以复制粘贴懂？

运行，生成datalink.exe文件后在此文件目录下鼠标右键+shift，打开powersell， 输入命令即可运行，要打开两个powershell，实现双工通信。
命令格式例如 .\datalink -d3 A  ,  .\datalink -d3 B
