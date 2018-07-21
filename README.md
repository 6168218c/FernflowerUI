# FernflowerUI
## Just a Java Decomplier with GUI and Decomplie Core called Fernflower

### 3.3 Update

### English Support
#### Thanks to @lokha for advice.

We have already implemented multi-language(in fact,only two languages are available,which are English(en-US) and Chinese(zh-CN)) feature.
But the application didn't provide the feature to toggle the language.The language is based on your Windows' UI language.
And, as you can see,we are not native English speakers,so please help us find out the grammer mistakes within the application.

#### This is what FernflowerUI is like (when your Windows' UI language is not Chinese) 
(In the image,we decomplied iChunUtil,please visit https://github.com/iChun/iChunUtil as respect):

![Using Image](https://raw.githubusercontent.com/6168218c/FernflowerUI/master/FernflowerUI_MFC/UsingImages/FernflowerUI3.3.jpg)

Now we've added the Font Choosing feature.You can choose the font you like in the "Settings" menu.The default Font is "Microsoft YaHei UI".
But what you can choose is only the font face and the font size.(Because the Syntax Highlight will make Java key word bold while making operators red, so we only enabled these two to avoid making Syntax Highlight terrible)

As metioned above,we have added some opinions:Quick decomplie and Clear cache.If you enable "Quick decomplie",when FernflowerUI is decompling using fernflower.jar,JRE will launch more than 2 threads to run the Jar.
The CPU utilization will be almost 100%,while the decomplie speed is increased.If you disable it, JRE will only use one CPU to decomplie.

The other opinion helps you to clear the decomplie cache that is stored in %LocalAppdata%\FernflowerUI (which is the same as %UserProFile%\Appdata\Local\FernflowerUI) to make FernflowerUI just read from the cache the next time you decomplie the same Jar file.FernflowerUI will begin clearing when all of the instances of the application are closed.
It can be useful when you don't want to delete the cache manually using Windows Explorer.

When you are viewing the decomplied Java Source file,you will see the line number in the left of the richedit window.
You can also press Ctrl+S to save the current (Active) Java Source file.


现在我们增加了字体选择功能。你可以在"设置"菜单中选择你喜爱的字体。默认字体是微软雅黑。但你只可以选择字体样式和字体大小（因为语法高亮会改变一些字体格式，因此我们只提供了这两个功能而避免使语法高亮太糟糕）

像上面提到的那样，我们增加了一些选项：快速反编译和清除缓存。如果你勾选“快速反编译”，fernflower.jar在反编译时会使用多核反编译，导致CPU占用率接近100%，但反编译速度会变快不少。

另外一个选项是"清除缓存"，这个选项帮助你清除%LocalAppdata%\FernflowerUI里的用来提升反编译速度的缓存。仅当所有FernflowerUI实例关闭后才会开始清除缓存。懒人党（比如该项目开发人）专属。

我们还实现了代码前行数的功能。在查看代码时，可以按Ctrl+S以保存当前阅览（活动的）的Java文件

Fixed:
If you have installed JRE but haven't set JAVA_HOME yet,now FernflowerUI won't warn that JRE hasn't been installed.The condition is that you can run "java -version" through CMD.
If you set “Hide the name of the known file extension" in your Folder Options , FernflowerUI can show the ClassView properly now.

修复:
如果你已经安装了JRE但没有设置JAVA_HOME，FernflowerUI现在不会提示”检测到JRE未安装“。条件是在CMD中可以运行"java -version"。
当电脑中设置"隐藏文件扩展名"时，FernflowerUI现在可以正常显示类视图及反编译后的Java文件。

## 另外,我们还在[McBBS](http://www.mcbbs.net/forum.php?mod=viewthread&tid=773809&page=1#pid12656797)上发布了这个项目。
