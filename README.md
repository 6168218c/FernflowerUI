# FernflowerUI
## Just a Java Decompiler with GUI and Decompile Core called Fernflower


### Download Link:[https://github.com/6168218c/FernflowerUI/releases/latest](https://github.com/6168218c/FernflowerUI/releases/latest)
### 下载链接:[https://github.com/6168218c/FernflowerUI/releases/latest](https://github.com/6168218c/FernflowerUI/releases/latest)


### *3.4 Changelog & Help*

#### This is what FernflowerUI is like (when your Windows' UI language is not Chinese) 
(In the image,we decompiled iChunUtil,please visit https://github.com/iChun/iChunUtil as respect):

![Image text](https://github.com/6168218c/FernflowerUI/blob/MFC/FernflowerUI_MFC/UsingImages/FernflowerUI3.4.jpg)

Now we've added the feature of finding a specific word in the .java file. Press Ctrl+F if you have opened a .java file , and the find dialog will be shown , allowing you find words in the current Java file.

We've also added the feature of saving either the whole Jar file or the Java file you are viewing , you can press Ctrl+S to save the whole Jar file , and press Ctrl+Shift+S to save the current Java file.
For the feature above , we've added the feature of saving the whole jar file automatically.You can enable the feature through the menu item that is in the submenu of "Settings".If the feature is enabled , you can find the decompiled jar file in the folder "\decompile" in the folder which the source jar file is in.
Besides , you can decompile the Jar file through the MRU(Most Recently Used) file list now.This time the application will decompile quickly thanks to the cache it saved.

Some small adjustments aren't metioned.

现在我们增加了字符串查找功能。当你已经打开一个Java文件后，按下Ctrl+F，“查找”对话框就会显示出来，允许你在当前Java文件中查找字符串。
我们也增加了保存整个Jar文件和保存当前Java文件的功能，你可以按Ctrl+S来保存整个Jar文件，也可以按Ctrl+Shift+S来保存当前的Java文件。
为了与上一个新增功能搭配，我们新增了自动保存功能。你可以在“设置”菜单中启用它。如果启用了该功能，反编译后的Jar文件将被自动保存到原Jar文件目录下的"\decompile"目录里。
另外，也可以通过MRU文件列表(最近文件列表)来反编译Jar文件。这次反编译速度会快很多，因为它在之前一次反编译中将结果存储到了缓存。

Fixed:
Now the application will check the existance of Java Runtime Environment asynchronously.
Improved the speed of the File View.

修复:
现在该程序会异步检测Java运行环境的存在。
提升了文件视图的速度。

#### P.S.: I forgot to save the source code of version 3.4,so the source code above belongs to test version of 3.4.1.

#### P.S.:由于我忘记保存3.4版本的源代码，这次发布的源代码是3.4.1的测试版的代码。

## 另外,我们还在[McBBS](http://www.mcbbs.net/thread-773809-1-1.html)上发布了这个项目。
