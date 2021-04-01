# ollydbg_plugin

+ 支持功能：**给函数加标签，重命名函数。**

  ![image-20210331174513564](https://cdn.jsdelivr.net/gh/smallzhong/new-picgo-pic-bed@master/image-20210331174513564.png)

  按下右键修改函数名称

  ![image-20210331174538089](https://cdn.jsdelivr.net/gh/smallzhong/new-picgo-pic-bed@master/image-20210331174538089.png)

  点击确定修改函数名称

  ![image-20210331174556621](https://cdn.jsdelivr.net/gh/smallzhong/new-picgo-pic-bed@master/image-20210331174556621.png)
  
+ 在内存窗口快速根据输入**选中相应的数据**并**保存到磁盘**

  首先选中需要复制的数据的第一个字节

  ![image-20210331210131967](https://cdn.jsdelivr.net/gh/smallzhong/new-picgo-pic-bed@master/image-20210331210131967.png)

  点击右键，选择“复制指定字节数”

  ![image-20210401153217012](https://cdn.jsdelivr.net/gh/smallzhong/new-picgo-pic-bed@master/image-20210401153217012.png)

  输入需要选择的字节数

  ![image-20210401153416512](https://cdn.jsdelivr.net/gh/smallzhong/new-picgo-pic-bed@master/image-20210401153416512.png)

  即可自动选中相应大小的数据。有时候比如需要跟踪程序调用 `writeFile` API写了什么东西到磁盘中时，就可以使用这个插件来选择并复制指定大小的数据。 

  ![image-20210401153718439](https://cdn.jsdelivr.net/gh/smallzhong/new-picgo-pic-bed@master/image-20210401153718439.png)

  在选中需要保存的数据之后选择“将选中的数据保存到文件”
  
  ![image-20210401153124752](https://cdn.jsdelivr.net/gh/smallzhong/new-picgo-pic-bed@master/image-20210401153124752.png)
  
  在弹出的另存为对话框中选择需要保存到的路径以及文件名
  
  ![image-20210401153652095](https://cdn.jsdelivr.net/gh/smallzhong/new-picgo-pic-bed@master/image-20210401153652095.png)
  
  点击保存弹出成功往xx写入xx字节数据即为保存成功
  
  ![image-20210401153703104](https://cdn.jsdelivr.net/gh/smallzhong/new-picgo-pic-bed@master/image-20210401153703104.png)

### TODO

- [x] 修改函数名称
- [x] 内存dump快速根据地址和大小选择相应的数据
- [x] 内存dump时可以快速把内存中的东西保存到磁盘