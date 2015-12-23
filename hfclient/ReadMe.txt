linelink.h
   与你的linelink.h相同
DisplayRpc.h
  这是你所要包含的头文件,用于显示神经网络结果
hfclient.cpp
  是使用DisplayRpc.h的例子


用DisplayRpc时,
第一步,
包含头文件,并定义全局变量
#include "DisplayRpc.h"
DisplayRpc display;
第二步
设置link数组
display.setLinkArray(link, link_num, width, height);
link是LineLink数组指针
link_num是LineLink数组大小
width是FPGA的X坐标的跨度, 不包含输入输出
height是FPGA的Y坐标的跨度
第三步
设置每个FPGA的输出值
display.showPower(power, power_num);
power是输出值的数组指针
power_num是输出值大小, 