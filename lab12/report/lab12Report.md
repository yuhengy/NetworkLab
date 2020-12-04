[TOC]

# 实验七实验报告
+ 杨宇恒 2017K8009929034

在结构划分上，ifaceIndex由IP层决定，但在MAC层才做出效果，这不合适。应该为每个iface提供一个MAC模块，这样，IP直接将请求发给对应的MAC模块。这是合理的，因为IP层调用MAC时需要提供MAC地址，而MAC地址与iface一一对应。

MOSPFD告诉IP层send all neighbour时，貌似不应该指定从那个IP出去，而让IP层处理广播。但其实一个IP出去可能连有hub，因此IP处理的广播在于，hub上不同IP地址都能收到它。这样看来，将IP层模块与每个MAC/Iface一一对应，而让路由表属于全局，也是合理的。