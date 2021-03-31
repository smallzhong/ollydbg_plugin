# ollydbg_plugin

+ 支持功能：**给函数加标签，重命名函数。**

  ![image-20210331174513564](https://cdn.jsdelivr.net/gh/smallzhong/new-picgo-pic-bed@master/image-20210331174513564.png)

  按下右键修改函数名称

  ![image-20210331174538089](https://cdn.jsdelivr.net/gh/smallzhong/new-picgo-pic-bed@master/image-20210331174538089.png)

  点击确定修改函数名称

  ![image-20210331174556621](https://cdn.jsdelivr.net/gh/smallzhong/new-picgo-pic-bed@master/image-20210331174556621.png)
  
+ 在内存窗口快速根据输入选中相应的数据

  首先选中需要复制的数据的第一个字节

  ![image-20210331210131967](https://cdn.jsdelivr.net/gh/smallzhong/new-picgo-pic-bed@master/image-20210331210131967.png)

  点击右键，选择“复制指定字节数”

  ![image-20210331210255802](https://cdn.jsdelivr.net/gh/smallzhong/new-picgo-pic-bed@master/image-20210331210255802.png)

  输入需要选择的字节数

  ![image-20210331210329466](https://cdn.jsdelivr.net/gh/smallzhong/new-picgo-pic-bed@master/image-20210331210329466.png)

  即可自动选中相应大小的数据。有时候比如需要跟踪程序调用 `writeFile` API写了什么东西到磁盘中时，就可以使用这个插件来选择并复制指定大小的数据。 

  ![image-20210331210336078](https://cdn.jsdelivr.net/gh/smallzhong/new-picgo-pic-bed@master/image-20210331210336078.png)

  

### TODO

- [x] 修改函数名称
- [x] 内存dump快速根据地址和大小选择相应的数据
- [ ] 内存dump时可以快速把内存中的东西保存到磁盘