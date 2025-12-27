# Cocos2d-x 用户手册

v2018.5.21

欢迎使用 Cocos2d-x 用户手册，本手册包含引擎的介绍，功能组件的使用方法以及引擎在多个平台的环境搭建。能够帮助您快速上手 Cocos2d-x！

## 特别推荐[](https://docs.cocos.com/cocos2d-x/manual/zh/www.cocos.com/other_node_types/#%E7%89%B9%E5%88%AB%E6%8E%A8%E8%8D%90)

- Cocos2d-x 3.17 已发布，请参阅 [版本发布说明](https://docs.cocos.com/cocos2d-x/manual/zh/www.cocos.com/other_node_types/www.cocos.com/1462)。
- 3.17 支持全平台的 CMake 构建，请参阅 [CMake 指南](https://docs.cocos.com/cocos2d-x/manual/zh/www.cocos.com/other_node_types/installation/CMake-Guide.html)

## 快速上手[](https://docs.cocos.com/cocos2d-x/manual/zh/www.cocos.com/other_node_types/#%E5%BF%AB%E9%80%9F%E4%B8%8A%E6%89%8B)

手册通过四个部分向您介绍 Cocos2d-x：新手入门部分，可以了解到 Cocos2d-x 引擎是什么、如何学习引擎、如何参与引擎开发，以及引擎中一些基本的概念；基本功能部分，着重介绍引擎中一些基础同时核心的组件如何使用，包括精灵、动作、场景。

进阶内容部分，是对引擎更近一步的阐述，包含如何进行一些高级控制、显示一些特殊效果；环境与工具部分，包含详细的开发环境搭建教程，以及一些引擎可利用的工具，比如使用 `cocos` 命令行。以下是手册的章节索引，可以帮助您快速定位。

- 新手入门：[了解引擎](https://docs.cocos.com/cocos2d-x/manual/zh/www.cocos.com/other_node_types/about/) / [基本概念](https://docs.cocos.com/cocos2d-x/manual/zh/www.cocos.com/other_node_types/basic_concepts/)
- 基本功能：[精灵](https://docs.cocos.com/cocos2d-x/manual/zh/www.cocos.com/other_node_types/sprites/) / [动作](https://docs.cocos.com/cocos2d-x/manual/zh/www.cocos.com/other_node_types/actions/) / [场景](https://docs.cocos.com/cocos2d-x/manual/zh/www.cocos.com/other_node_types/scenes/) / [UI 组件](https://docs.cocos.com/cocos2d-x/manual/zh/www.cocos.com/other_node_types/ui_components/)
- 进阶内容：[特殊节点对象](https://docs.cocos.com/cocos2d-x/manual/zh/www.cocos.com/other_node_types/other_node_types/) / [事件分发机制](https://docs.cocos.com/cocos2d-x/manual/zh/www.cocos.com/other_node_types/event_dispatcher/) / [3D 支持](https://docs.cocos.com/cocos2d-x/manual/zh/www.cocos.com/other_node_types/3d/) / [使用脚本](https://docs.cocos.com/cocos2d-x/manual/zh/www.cocos.com/other_node_types/scripting/) / [物理引擎](https://docs.cocos.com/cocos2d-x/manual/zh/www.cocos.com/other_node_types/physics/) / [音乐和音效](https://docs.cocos.com/cocos2d-x/manual/zh/www.cocos.com/other_node_types/audio/) / [高级话题](https://docs.cocos.com/cocos2d-x/manual/zh/www.cocos.com/other_node_types/advanced_topics/)
- 环境与工具：[环境搭建](https://docs.cocos.com/cocos2d-x/manual/zh/www.cocos.com/other_node_types/installation/) / [引擎工具](https://docs.cocos.com/cocos2d-x/manual/zh/www.cocos.com/other_node_types/editors_and_tools/cocosCLTool.html)

在手册的使用过程中，您大可不必按照目录的顺序，一章一章的阅读，完全可以跳跃，比如阅读完新手入门部分后，直接进入环境搭建章节，按照教程在自己 PC 上搭建好开发环境，完成后，一边看源码一边看手册，或许这样能有更好的学习效果。您也可以把本文档当做一个查询手册，将你想查询的直接输入到左上角的全局搜索框，回车一下，结果将立刻显现在页面中。

# 导演(Director)

Cocos2d-x 使用导演的概念，这个导演和电影制作过程中的导演一样！导演控制电影制作流程，指导团队完成各项任务。在使用 Cocos2d-x 开发游戏的过程中，你可以认为自己是执行制片人，告诉 **导演(Director)** 该怎么办！一个常见的 `Director` 任务是控制场景替换和转换。 `Director`是一个共享的单例对象，可以在代码中的任何地方调用。

这是一个典型的游戏流程实例。当您的游戏设计好时，`Director` 就负责场景的转换：

![](https://docs.cocos.com/cocos2d-x/manual/en/basic_concepts/basic_concepts-img/scenes.png)

你是你的游戏的导演。你决定着发生什么，何时发生，如何发生。

# 场景(Scene)

在游戏开发过程中，你可能需要一个主菜单，几个关卡和一个结束场景。如何组织所有这些分开的部分？使用 **场景(Scene)** ！当你想到喜欢的电影时，你能观察到它是被分解为不同场景或不同故事线。现在我们对游戏开发应用这个相同的思维过程，你应该很容易就能想出几个场景。

来看一张熟悉的图片：

![](https://docs.cocos.com/cocos2d-x/manual/en/basic_concepts/basic_concepts-img/2n_main.png)

这是一个主菜单场景，这个场景是由很多小的对象拼接而成，所有的对象组合在一起，形成了最终的结果。场景是被 **渲染器(renderer)** 画出来的。渲染器负责渲染精灵和其它的对象进入屏幕。为了更好的理解这个过程，我们需要讨论一下 **场景图**。

## 场景图(Scene Graph)[](https://docs.cocos.com/cocos2d-x/manual/zh/basic_concepts/scene.html#%E5%9C%BA%E6%99%AF%E5%9B%BEscene-graph)

场景图(Scene Graph)是一种安排场景内对象的数据结构，它把场景内所有的 **节点(Node)** 都包含在一个 **树(tree)** 上。(场景图虽然叫做"图"，但实际使用一个树结构来表示)。

![](https://docs.cocos.com/cocos2d-x/manual/en/basic_concepts/basic_concepts-img/tree.jpg "Simple Tree")

听起来这好像很复杂，可能你会问，我为什么要关注这个技术细节，Cocos2d-x 值得我研究的这么深入吗？值得！这个对你真正了解渲染器是如何绘制场景的非常重要。

当你开发游戏的时候，你会添加一些节点，精灵和动画到一个场景中，你期望的是每一个添加的对象都能被正确的展示，可是如果有个对象没有被展示呢？可能你错误的把这个对象隐藏到背景中了。怎么办？别着急，这是个小问题，停下来，拿出一张纸，把场景图画出来，你肯定能很容易的发现错误。

既然场景图是一个树结构，你就能遍历它，Cocos2d-x 使用 `中序遍历`，先遍历左子树，然后根节点，最后是右子树。中序遍历下图的节点，能得到 `A, B, C, D, E, F, G, H, I` 这样的序列。

![](https://docs.cocos.com/cocos2d-x/manual/en/basic_concepts/basic_concepts-img/in-order-walk.png "in-order walk")

初步了解了场景图，让我们看一下这个游戏场景。

![](https://docs.cocos.com/cocos2d-x/manual/en/basic_concepts/basic_concepts-img/2n_main.png)

分解这个场景，看一下它有哪些元素，这些最终会被渲染为一个树。

![](https://docs.cocos.com/cocos2d-x/manual/en/basic_concepts/basic_concepts-img/2n_mainScene-sceneGraph.png)

另一点要考虑的是，**z-order** 为负的元素，z-order 为负的节点会被放置在左子树，非负的节点会被放在右子树。实际开发的过程中，你可以按照任意顺序添加对象，他们会按照你指定的 z-order 自动排序。

![](https://docs.cocos.com/cocos2d-x/manual/en/basic_concepts/basic_concepts-img/layers.png)

如上图，左侧的场景是由很多节点对象组成的，他们根据被指定的 z-order 相互叠加。在 Cocos2d-x 中，通过 `Scene` 的 `addChild()` 方法构建场景图.

```
// Adds a child with the z-order of -2, that means
// it goes to the "left" side of the tree (because it is negative)
scene->addChild(title_node, -2);

// When you don't specify the z-order, it will use 0
scene->addChild(label_node);

// Adds a child with the z-order of 1, that means
// it goes to the "right" side of the tree (because it is positive)
scene->addChild(sprite_node, 1);
```

渲染时 `z-order` 值大的节点对象会后绘制，值小的节点对象先绘制。如果两个节点对象的绘制范围有重叠，`z-order` 值大的可能会覆盖 `z-order` 值小的。

# 精灵(Sprite)

不知你是否意识到，所有的游戏都有 **精灵(Sprite)** 对象，精灵是您在屏幕上移动的对象，它能被控制。你喜欢玩的游戏中主角可能就是一个精灵，我知道你在想是不是每个图形对象都是一个精灵，不是的，为什么? 如果你能控制它，它才是一个精灵，如果无法控制，那就只是一个节点(Node)。

看下面的图片，我们来指出一下，哪个是精灵(Sprite)，哪个是节点(Node)。

![](https://docs.cocos.com/cocos2d-x/manual/en/basic_concepts/basic_concepts-img/2n_main_sprites_nodes.png)

精灵在所有游戏中都很重要，每个游戏都有这样的情景：一个舞台，上面站着一个某种形式的主角，那主角就是精灵。`Sprite` 很容易被创建，它有一些可以被配置的属性，比如：位置，旋转角度，缩放比例，透明度，颜色 等等。

```
// This is how to create a sprite
auto mySprite = Sprite::create("mysprite.png");

// this is how to change the properties of the sprite
mySprite->setPosition(Vec2(500, 0));

mySprite->setRotation(40);

mySprite->setScale(2.0); // sets both the scale of the X and Y axis uniformly

mySprite->setAnchorPoint(Vec2(0, 0));
```

让我们举例说明每个属性的含义，思考下面不同截图中精灵的区别：

![](https://docs.cocos.com/cocos2d-x/manual/en/basic_concepts/basic_concepts-img/2n_level1_action_start.png)

设置位置 `mySprite->setPosition(Vec2(500, 0));`：

![](https://docs.cocos.com/cocos2d-x/manual/en/basic_concepts/basic_concepts-img/2n_level1_action_end.png)

现在这个精灵的位置就变成了，我们设置的新地方。

设置旋转角度 `mySprite->setRotation(40);`：

![](https://docs.cocos.com/cocos2d-x/manual/en/basic_concepts/basic_concepts-img/2n_level1_action_end_rotation.png)

可以发现这个精灵已经被旋转了设置的角度

设置缩放比例 `mySprite->setScale(2.0);`：

![](https://docs.cocos.com/cocos2d-x/manual/en/basic_concepts/basic_concepts-img/2n_level1_action_end_scale.png)

看到了精灵的大小，由于我们设置缩放而变化了。

我们再来说一下 **锚点(anchor point)** ，所有的节点(Node)对象都有锚点值，`Sprite` 是 `Node` 的子类，自然也具有锚点。锚点是节点对象在计算坐标位置时的一个基准点。

以我们刚才的展示的精灵为例，设置锚点(0,0)：

```
mySprite->setAnchorPoint(Vec2(0, 0));
```

精灵的左下角就变为了 `setPosition()` 调用，计算坐标的基础。再看看其它的锚点效果：

![](https://docs.cocos.com/cocos2d-x/manual/en/basic_concepts/basic_concepts-img/2n_level1_anchorpoint_0_0.png) ![](https://docs.cocos.com/cocos2d-x/manual/en/basic_concepts/basic_concepts-img/smallSpacer.png) ![](https://docs.cocos.com/cocos2d-x/manual/en/basic_concepts/basic_concepts-img/2n_level1_anchorpoint_05_05.png) ![](https://docs.cocos.com/cocos2d-x/manual/en/basic_concepts/basic_concepts-img/smallSpacer.png) ![](https://docs.cocos.com/cocos2d-x/manual/en/basic_concepts/basic_concepts-img/2n_level1_anchorpoint_1_1.png)

注意每张图片中的红点，红点表示锚点的位置。

正如你所看到的那样，锚点对于确定节点对象的位置是非常有用的，你可以在你的游戏中动态的调整锚点值以实现你想要的效果。

现在我们可以静态调整精灵的各个方面，但是你要想这些属性按照时间自动变化该如何做呢? 继续阅读，很快你就会有答案。

# 动作(Action)

创建一个场景，在场景里面增加精灵只是完成一个游戏的第一步，接下来我们要解决的问题就是，怎么让精灵动起来。**动作(Action)** 就是用来解决这个问题的，它可以让精灵在场景中移动，如从一个点移动到另外一个点。你还可以创建一个动作 **序列(Sequence)** ，让精灵按照这个序列做连续的动作，在动作过程中你可以改变精灵的位置，旋转角度，缩放比例等等。

在 [代码示例](https://github.com/chukong/programmers-guide-samples) 中，有对应的章节，执行效果是这样：

![](https://docs.cocos.com/cocos2d-x/manual/en/basic_concepts/basic_concepts-img/2n_level1_action_start.png)

5s 后，精灵移动到了一个新的位置：

![](https://docs.cocos.com/cocos2d-x/manual/en/basic_concepts/basic_concepts-img/2n_level1_action_end.png)

`Action` 对象的创建：

```
auto mySprite = Sprite::create("Blue_Front1.png");

// Move a sprite 50 pixels to the right, and 10 pixels to the top over 2 seconds.
auto moveBy = MoveBy::create(2, Vec2(50,10));
mySprite->runAction(moveBy);

// Move a sprite to a specific location over 2 seconds.
auto moveTo = MoveTo::create(2, Vec2(50,10));
mySprite->runAction(moveTo);
```


# 节点关系

Cocos2d-x 的 **节点关系**，是被附属和附属的关系，就像数据结构中的父子关系，如果两个节点被添加到一个父子关系中，那么父节点的属性变化会被自动应用到子节点中。想一下处于父子关系中的精灵有什么特性。

![](https://docs.cocos.com/cocos2d-x/manual/en/basic_concepts/basic_concepts-img/2n_parent.png)

这三个精灵被添加到了一个父子关系中，当父精灵(被其它精灵附属的精灵)设置了旋转角度之后，子精灵也会自动做同样的改变：

![](https://docs.cocos.com/cocos2d-x/manual/en/basic_concepts/basic_concepts-img/2n_parent_rotation.png)

```
auto myNode = Node::create();

// rotating by setting
myNode->setRotation(50);
```

和旋转角度一样，如果你改变了父精灵的缩放比例，子精灵也会做同样的改变：

![](https://docs.cocos.com/cocos2d-x/manual/en/basic_concepts/basic_concepts-img/2n_parent_scaled.png)

```
auto myNode = Node::create();

// scaling by setting
myNode->setScale(2.0); // scales uniformly by 2.0
```

需要注意的是，不是所有的父节点属性都会被自动应用到子节点，如改变父节点的锚点只会影响转换效果(_比例缩放，位置变化，角度旋转，变形等_)，不会影响子节点锚点，子节点的锚点总会是左下角 (0,0)。

# 日志输出

有时，在你的游戏正在运行的时候，为了了解程序的运行过程或是为了查找一个 BUG，你想看到一些运行时信息，可以! 这个需求引擎已经考虑到了，使用 `log()` 可以把信息输出到控制台，这样使用：

```
// a simple string
log("This would be outputted to the console");

// a string and a variable
string s = "My variable";
log("string is %s", s);

// a double and a variable
double dd = 42;
log("double is %f", dd);

// an integer and a variable
int i = 6;
log("integer is %d", i);

// a float and a variable
float f = 2.0f;
log("float is %f", f);

// a bool and a variable
bool b = true;
if (b == true)
    log("bool is true");
else
    log("bool is false");
```

对于使用 C++ 进行游戏开发的用户来说，可能想使用 `std::cout` 而不用 `log()`，实际上 `log()` 更易于使用，它格式化复杂的输出信息更简单。

# 精灵(Sprite)

之前我们提到，精灵是屏幕上移动的对象，它能被控制。你喜欢玩的游戏中主角可能就是一个精灵，我知道你在想是不是每个图形对象都是一个精灵，不是的，为什么? 如果你能控制它，它才是一个精灵，如果无法控制，那就只是一个节点(Node)。

准确的说，**精灵(Sprite)** 是一个能通过改变自身的属性：角度，位置，缩放，颜色等，变成可控制动画的 2D 图像。

# 精灵的创建

可以使用一张图像来创建精灵，_PNG, JPEG, TIFF, WebP_, 这几个格式都可以。当然也有一些其它的方式可以创建精灵，如使用 **图集** 创建，通过 **精灵缓存** 创建，我们会一个一个的讨论。本节介绍通过图像创建精灵。

## 使用图像创建[](https://docs.cocos.com/cocos2d-x/manual/zh/sprites/creating.html#%E4%BD%BF%E7%94%A8%E5%9B%BE%E5%83%8F%E5%88%9B%E5%BB%BA)

`Sprite` 能用一个特定的图像去创建:

```
auto mySprite = Sprite::create("mysprite.png");
```

![](https://docs.cocos.com/cocos2d-x/manual/en/sprites/sprites-img/i1.png)

上面直接使用了 **mysprite.png** 图像来创建精灵。精灵会使用整张图像，图像是多少的分辨率，创建出来的精灵就是多少的分辨率。比如图像是 200 x 200，`Sprite` 也是 200 x 200。

### 使用矩形[](https://docs.cocos.com/cocos2d-x/manual/zh/sprites/creating.html#%E4%BD%BF%E7%94%A8%E7%9F%A9%E5%BD%A2)

上一个例子，精灵和原始图像的尺寸一致。但是如果你想创建一个尺寸只有原始图像一部分的精灵，那你可以在创建的时候指定一个矩形，指定矩形需要四个值，初始 x 坐标，初始 y 坐标，矩形宽，矩形高。

```
auto mySprite = Sprite::create("mysprite.png", Rect(0,0,40,40));
```

![](https://docs.cocos.com/cocos2d-x/manual/en/sprites/sprites-img/i4.png)

矩形的初始坐标，从图形的左上角开始算，即左上角的坐标是 (0, 0)，不是从左下角。因此结果精灵是图像左上角的一小块，从左上角开始算起，40 x 40 的大小。

如果你没指定一个矩形，Cocos2d-x 引擎就会自动使用这个图像全部的宽和高，看下面的例子，如果你把矩形的宽高指定为图像的宽高，矩形的初始坐标指定为 (0, 0)，那这就和第一种情况的效果是完全一样的。

```
auto mySprite = Sprite::create("mysprite.png");

auto mySprite = Sprite::create("mysprite.png", Rect(0,0,200,200));
```


# 使用图集

**图集(Sprite Sheet)** 是通过专门的工具将多张图片合并成一张大图，并通过 plist 等格式的文件索引的资源，使用图集比使用多个独立图像占用的磁盘空间更少，还会有更好的性能。这种方式已经是游戏行业中提高游戏性能的标准方法之一。

在使用图集时，首先将其全部加载到 `SpriteFrameCache` 中，`SpriteFrameCache` 是一个全局的缓存类，缓存了添加到其中的 `SpriteFrame` 对象，提高了精灵的访问速度。`SpriteFrame` 只加载一次，后续一直保存在 `SpriteFrameCache` 中。

示例：

![](https://docs.cocos.com/cocos2d-x/manual/en/sprites/sprites-img/3_1.png "example SpriteSheet")

单看这个图集，似乎很难分析出什么，让我们对比一下：

![](https://docs.cocos.com/cocos2d-x/manual/en/sprites/sprites-img/spritesheet.png "example SpriteSheet")

这就很容易看出来，它至少完成了将多个图像素材合为一个，同时减少了磁盘空间占用。

继续来看如何在代码中使用。

## 加载图集[](https://docs.cocos.com/cocos2d-x/manual/zh/sprites/spritesheets.html#%E5%8A%A0%E8%BD%BD%E5%9B%BE%E9%9B%86)

获取到 `SpriteFrameCache` 的实例，把图集添加到实例中。

```
// load the Sprite Sheet
auto spritecache = SpriteFrameCache::getInstance();

// the .plist file can be generated with any of the tools mentioned below
spritecache->addSpriteFramesWithFile("sprites.plist");
```

这样我们就完成了，将一个图集添加到 `SpriteFrameCache` 中，现在我们就能利用这个对象创建精灵了！

## 创建图集[](https://docs.cocos.com/cocos2d-x/manual/zh/sprites/spritesheets.html#%E5%88%9B%E5%BB%BA%E5%9B%BE%E9%9B%86)

手动创建图集资源是一个单调乏味的过程，幸运的是有一些工具能帮助我们自动创建，下面是推荐的几个工具：

- [Texture Packer](https://www.codeandweb.com/texturepacker)
- [Zwoptex](https://www.zwopple.com/zwoptex/)
- [ShoeBox](http://renderhjs.net/shoebox/)
- [Sprite Sheet Packer](http://amakaseev.github.io/sprite-sheet-packer/)

其中 **Texture Packer** 有一个专门为 Cocos2d-x 写的图集创建指南。[传送门](https://www.codeandweb.com/texturepacker/tutorials/animations-and-spritesheets-in-cocos2d-x)

# 使用精灵缓存

精灵缓存是 Cocos2d-x 为了提高精灵的访问速度，提供的一个精灵的缓存机制。

我们可以创建一个精灵并把精灵放到精灵的缓存对象 **`SpriteFrameCache`** 中：

```
// Our .plist file has names for each of the sprites in it.  We'll grab
// the sprite named, "mysprite" from the sprite sheet:
auto mysprite = Sprite::createWithSpriteFrameName("mysprite.png");
```

![](https://docs.cocos.com/cocos2d-x/manual/en/sprites/sprites-img/i3.png)

相对的，我们也可以从精灵的缓存对象 `SpriteFrameCache` 访问一个精灵，访问方法是先从缓存对象中获取对应的 `SpriteFrame`，然后从 `SpriteFrame`创建精灵，方法：

```
// this is equivalent to the previous example,
// but it is created by retrieving the SpriteFrame from the cache.
auto newspriteFrame = SpriteFrameCache::getInstance()->getSpriteFrameByName("Blue_Front1.png");
auto newSprite = Sprite::createWithSpriteFrame(newspriteFrame);
```

![](https://docs.cocos.com/cocos2d-x/manual/en/sprites/sprites-img/i3.png)

# 精灵的控制

在创建完精灵后，现在你能试着修改精灵的属性去控制它了。

创建精灵：

```
auto mySprite = Sprite::create("mysprite.png");
```

![](https://docs.cocos.com/cocos2d-x/manual/en/sprites/sprites-img/i1.png)

## 锚点[](https://docs.cocos.com/cocos2d-x/manual/zh/sprites/manipulation.html#%E9%94%9A%E7%82%B9)

锚点确定了精灵对象在计算坐标位置的一个基准点，这个点是精灵内部的点，锚点影响精灵的缩放，旋转，倾斜这种转换，不影响颜色，透明度这种属性。锚点使用的坐标系以左下角为原点 (0, 0)，在你设置锚点的值时，要注意到这一点。默认情况下，所有的节点对象锚点是 (0.5, 0.5)。

设置锚点：

```
// DEFAULT anchor point for all Sprites
mySprite->setAnchorPoint(0.5, 0.5);

// bottom left
mySprite->setAnchorPoint(0, 0);

// top left
mySprite->setAnchorPoint(0, 1);

// bottom right
mySprite->setAnchorPoint(1, 0);

// top right
mySprite->setAnchorPoint(1, 1);
```

观察下面的图，感受锚点对精灵位置的影响，_红点是旁边精灵的锚点_。

![](https://docs.cocos.com/cocos2d-x/manual/en/sprites/sprites-img/i6.png)

## 位置[](https://docs.cocos.com/cocos2d-x/manual/zh/sprites/manipulation.html#%E4%BD%8D%E7%BD%AE)

精灵的位置受锚点影响，看一下这个具体是怎样的，以红线红点为环境参考，看精灵的不同位置。注意，我们设置了锚点值，精灵的位置变化了，我们并没有使用 `setPosition()` 方法设置精灵的位置。

![](https://docs.cocos.com/cocos2d-x/manual/en/sprites/sprites-img/i9.png)

当我们想设置一个精灵的位置时，主要是使用 `setPosition()` 方法，只有想改变精灵与基准坐标点的相对位置时，才考虑使用 `setAnchorPoint()` 设置锚点。

```
// position a sprite to a specific position of x = 100, y = 200.
mySprite->setPosition(Vec2(100, 200));
```

## 旋转[](https://docs.cocos.com/cocos2d-x/manual/zh/sprites/manipulation.html#%E6%97%8B%E8%BD%AC)

通过 `setRotation()` 方法，设置一个角度值可以控制精灵的旋转，正值精灵顺时针旋转，负值精灵逆时针旋转，默认位置的角度值是 0.0。

```
// rotate sprite by +20 degrees
mySprite->setRotation(20.0f);

// rotate sprite by -20 degrees
mySprite->setRotation(-20.0f);

// rotate sprite by +60 degrees
mySprite->setRotation(60.0f);

// rotate sprite by -60 degrees
mySprite->setRotation(-60.0f);
```

![](https://docs.cocos.com/cocos2d-x/manual/en/sprites/sprites-img/i8.png)

## 缩放[](https://docs.cocos.com/cocos2d-x/manual/zh/sprites/manipulation.html#%E7%BC%A9%E6%94%BE)

通过 `setScale()` 方法控制精灵的缩放。可以控制精灵水平缩放，垂直缩放，也可以整体缩放。默认水平和竖直的缩放值都是 1.0。

```
// increases X and Y size by 2.0 uniformly
mySprite->setScale(2.0);

// increases just X scale by 2.0
mySprite->setScaleX(2.0);

// increases just Y scale by 2.0
mySprite->setScaleY(2.0);
```

![](https://docs.cocos.com/cocos2d-x/manual/en/sprites/sprites-img/i5.png)

## 倾斜[](https://docs.cocos.com/cocos2d-x/manual/zh/sprites/manipulation.html#%E5%80%BE%E6%96%9C)

通过 `setSkewX()` 控制精灵的倾斜度，可以控制精灵水平倾斜，竖直倾斜，或者水平竖直同时倾斜，默认水平和竖直的倾斜值都是 0.0。

```
// adjusts the X skew by 20.0
mySprite->setSkewX(20.0f);

// adjusts the Y skew by 20.0
mySprite->setSkewY(20.0f);
```

![](https://docs.cocos.com/cocos2d-x/manual/en/sprites/sprites-img/i7.png)

## 颜色[](https://docs.cocos.com/cocos2d-x/manual/zh/sprites/manipulation.html#%E9%A2%9C%E8%89%B2)

通过 `setColor()` 控制精灵的颜色。将一个 _RGB_ 值设置到 `Color3B` 对象，调用精灵的 `setColor()` ，就能完成精灵颜色的设置。_RGB_ 是三个从 0-255 的值，三个值分别代表红绿蓝的颜色深度，数值越大，颜色越深。特别的 RGB(255, 255, 255) 是白色。如果你不想自己指定 _RGB_ 的三个值，也可以使用 Cocos2d-x 提供的预定义颜色，比如: `Color3B::White`，`Color3B::Red`。

```
// set the color by passing in a pre-defined Color3B object.
mySprite->setColor(Color3B::WHITE);

// Set the color by passing in a Color3B object.
mySprite->setColor(Color3B(255, 255, 255)); // Same as Color3B::WHITE
```

![](https://docs.cocos.com/cocos2d-x/manual/en/sprites/sprites-img/i10.png)

## 透明度[](https://docs.cocos.com/cocos2d-x/manual/zh/sprites/manipulation.html#%E9%80%8F%E6%98%8E%E5%BA%A6)

精灵的透明度可以通过 `setOpacity()` 传入一个特定的值来设置，这个值的范围是 0-255，数值越大透明度越低，255 代表完全不透明，0 代表完全透明。

```
// Set the opacity to 30, which makes this sprite 11.7% opaque.
// (30 divided by 256 equals 0.1171875...)
mySprite->setOpacity(30);
```

![](https://docs.cocos.com/cocos2d-x/manual/en/sprites/sprites-img/i11.png)

# 多边形精灵

**多边形精灵(Polygon Sprite)** 也是一个精灵，同样是为了展示一个可以被控制的图像，但是和普通精灵的区别是，普通精灵在绘图处理中被分为了两个三角形，多边形精灵则是被分为了一系列三角形。

## 为什么要使用多边形精灵[](https://docs.cocos.com/cocos2d-x/manual/zh/sprites/polygon.html#%E4%B8%BA%E4%BB%80%E4%B9%88%E8%A6%81%E4%BD%BF%E7%94%A8%E5%A4%9A%E8%BE%B9%E5%BD%A2%E7%B2%BE%E7%81%B5)

**提高性能**!

要深入分析这个是如何提高性能的，会需要很多和像素填充率有关的技术术语。幸好本节是入门性质的文档，能让大家理解多边形精灵比普通精灵性能好就可以了，不用讨论特定宽高矩形绘制时的性能问题。

![](https://docs.cocos.com/cocos2d-x/manual/en/sprites/sprites-img/polygonsprite.png)

注意左右两种情况的不同。

左侧，是一个典型的精灵绘制时的处理，精灵被处理成一个有两个三角形组成的矩形。

右侧，是一个多边形精灵绘制时的处理，精灵被处理成一系列小的三角形。

显然可以看到，右侧多边形精灵需要绘制的像素数量比左侧精灵需要的像素数量更小，但是由于划分了多个三角形出现了更多的顶点，由于在现代的图形处理中，一般绘制定点比绘制像素消耗的性能少。所以多边形精灵的性能更好，实际的测试结果也验证了这一点。

## AutoPolygon[](https://docs.cocos.com/cocos2d-x/manual/zh/sprites/polygon.html#autopolygon)

**`AutoPolygon`** 是一个工具类，它可以在程序运行时，通过跟踪关键点和三角测量，将一个矩形图像划分成一系列小三角形块。

首先将图像资源传入 `AutoPolygon` 进行处理，然后我们使用它生成的对象进行精灵的创建就能得到多边形精灵。

```
// Generate polygon info automatically.
auto pinfo = AutoPolygon::generatePolygon("filename.png");

// Create a sprite with polygon info.
auto sprite = Sprite::create(pinfo);
```

# 动作(Action)

**动作(Action)** 的功能就和字面含义一样，它通过改变一个 `Node` 对象的属性，让它表现出某种动作。动作对象能实时的改变 `Node` 的属性，任何一个对象只要它是 `Node` 的子类都能被改变。比如，你能通过动作对象把一个精灵从一个位置移动到另一个位置。

通过 `MoveTo` 和 `MoveBy` 方法:

```
// Move sprite to position 50,10 in 2 seconds.
auto moveTo = MoveTo::create(2, Vec2(50, 10));
mySprite1->runAction(moveTo);

// Move sprite 20 points to right in 2 seconds
auto moveBy = MoveBy::create(2, Vec2(20,0));
mySprite2->runAction(moveBy);
```

## By 和 To 的区别[](https://docs.cocos.com/cocos2d-x/manual/zh/actions/#by-%E5%92%8C-to-%E7%9A%84%E5%8C%BA%E5%88%AB)

你能注意到，每一个动作都会有两个方法 **By** 和 **To**。两种方法方便你在不同的情况使用，**By** 算的是相对于节点对象的当前位置，**To** 算的是绝对位置，不考虑当前节点对象在哪。如果你想动作的表现是相对于 `Node` 当前位置的，就用 **By**，相对的想让动作的表现是按照坐标的绝对位置就用 **To**。看一个例子：

```
auto mySprite = Sprite::create("mysprite.png");
mySprite->setPosition(Vec2(200, 256));

// MoveBy - lets move the sprite by 500 on the x axis over 2 seconds
// MoveBy is relative - since x = 200 + 500 move = x is now 700 after the move
auto moveBy = MoveBy::create(2, Vec2(500, mySprite->getPositionY()));

// MoveTo - lets move the new sprite to 300 x 256 over 2 seconds
// MoveTo is absolute - The sprite gets moved to 300 x 256 regardless of
// where it is located now.
auto moveTo = MoveTo::create(2, Vec2(300, mySprite->getPositionY()));

// Delay - create a small delay
auto delay = DelayTime::create(1);

auto seq = Sequence::create(moveBy, delay, moveTo, nullptr);

mySprite->runAction(seq);
```

![](https://docs.cocos.com/cocos2d-x/manual/en/actions/actions-img/i0.png)

# 基本动作

基本动作通常都是单一的动作，用来完成一个简单的目标。下面通过简单的示例来介绍常见的基本动作。

## 移动[](https://docs.cocos.com/cocos2d-x/manual/zh/actions/basic.html#%E7%A7%BB%E5%8A%A8)

使用 `MoveTo` `MoveBy` 完成节点对象在一个设置的时间后移动。

```
auto mySprite = Sprite::create("mysprite.png");

// Move a sprite to a specific location over 2 seconds.
auto moveTo = MoveTo::create(2, Vec2(50, 0));

mySprite->runAction(moveTo);

// Move a sprite 50 pixels to the right, and 0 pixels to the top over 2 seconds.
auto moveBy = MoveBy::create(2, Vec2(50, 0));

mySprite->runAction(moveBy);
```

![](https://docs.cocos.com/cocos2d-x/manual/en/actions/actions-img/i1.png)

## 旋转[](https://docs.cocos.com/cocos2d-x/manual/zh/actions/basic.html#%E6%97%8B%E8%BD%AC)

使用 `RotateTo` `RotateBy` 完成节点对象在一个设置的时间后顺时针旋转指定角度。

```
auto mySprite = Sprite::create("mysprite.png");

// Rotates a Node to the specific angle over 2 seconds
auto rotateTo = RotateTo::create(2.0f, 40.0f);
mySprite->runAction(rotateTo);

// Rotates a Node clockwise by 40 degree over 2 seconds
auto rotateBy = RotateBy::create(2.0f, 40.0f);
mySprite->runAction(rotateBy);
```

![](https://docs.cocos.com/cocos2d-x/manual/en/actions/actions-img/i3.png)

## 缩放[](https://docs.cocos.com/cocos2d-x/manual/zh/actions/basic.html#%E7%BC%A9%E6%94%BE)

使用 `ScaleBy` `ScaleTo` 完成节点对象的比例缩放。

```
auto mySprite = Sprite::create("mysprite.png");

// Scale uniformly by 3x over 2 seconds
auto scaleBy = ScaleBy::create(2.0f, 3.0f);
mySprite->runAction(scaleBy);

// Scale X by 5 and Y by 3x over 2 seconds
auto scaleBy = ScaleBy::create(2.0f, 3.0f, 3.0f);
mySprite->runAction(scaleBy);

// Scale to uniformly to 3x over 2 seconds
auto scaleTo = ScaleTo::create(2.0f, 3.0f);
mySprite->runAction(scaleTo);

// Scale X to 5 and Y to 3x over 2 seconds
auto scaleTo = ScaleTo::create(2.0f, 3.0f, 3.0f);
mySprite->runAction(scaleTo);
```

![](https://docs.cocos.com/cocos2d-x/manual/en/actions/actions-img/i4.png)

### 淡入淡出[](https://docs.cocos.com/cocos2d-x/manual/zh/actions/basic.html#%E6%B7%A1%E5%85%A5%E6%B7%A1%E5%87%BA)

使用 `FadeIn` `FadeOut` 完成节点对象的淡入，淡出。 `FadeIn` 修改节点对象的透明度属性，从完全透明到完全不透明，`FadeOut` 相反。

```
auto mySprite = Sprite::create("mysprite.png");

// fades in the sprite in 1 seconds
auto fadeIn = FadeIn::create(1.0f);
mySprite->runAction(fadeIn);

// fades out the sprite in 2 seconds
auto fadeOut = FadeOut::create(2.0f);
mySprite->runAction(fadeOut);
```

![](https://docs.cocos.com/cocos2d-x/manual/en/actions/actions-img/i2.png)

## 色彩混合[](https://docs.cocos.com/cocos2d-x/manual/zh/actions/basic.html#%E8%89%B2%E5%BD%A9%E6%B7%B7%E5%90%88)

使用 `TintTo` `TintBy`，将一个实现了 `NodeRGB` 协议的节点对象进行色彩混合。

```
auto mySprite = Sprite::create("mysprite.png");

// Tints a node to the specified RGB values
auto tintTo = TintTo::create(2.0f, 120.0f, 232.0f, 254.0f);
mySprite->runAction(tintTo);

// Tints a node BY the delta of the specified RGB values.
auto tintBy = TintBy::create(2.0f, 120.0f, 232.0f, 254.0f);
mySprite->runAction(tintBy);
```

![](https://docs.cocos.com/cocos2d-x/manual/en/actions/actions-img/i5.png)

## 帧动画[](https://docs.cocos.com/cocos2d-x/manual/zh/actions/basic.html#%E5%B8%A7%E5%8A%A8%E7%94%BB)

使用 `Animate` 对象可以很容易的通过每隔一个短暂时间进行图像替代的方式，实现一个翻页效果。下面是一个例子：

```
auto mySprite = Sprite::create("mysprite.png");

// now lets animate the sprite we moved
Vector<SpriteFrame*> animFrames;
animFrames.reserve(12);
animFrames.pushBack(SpriteFrame::create("Blue_Front1.png", Rect(0,0,65,81)));
animFrames.pushBack(SpriteFrame::create("Blue_Front2.png", Rect(0,0,65,81)));
animFrames.pushBack(SpriteFrame::create("Blue_Front3.png", Rect(0,0,65,81)));
animFrames.pushBack(SpriteFrame::create("Blue_Left1.png", Rect(0,0,65,81)));
animFrames.pushBack(SpriteFrame::create("Blue_Left2.png", Rect(0,0,65,81)));
animFrames.pushBack(SpriteFrame::create("Blue_Left3.png", Rect(0,0,65,81)));
animFrames.pushBack(SpriteFrame::create("Blue_Back1.png", Rect(0,0,65,81)));
animFrames.pushBack(SpriteFrame::create("Blue_Back2.png", Rect(0,0,65,81)));
animFrames.pushBack(SpriteFrame::create("Blue_Back3.png", Rect(0,0,65,81)));
animFrames.pushBack(SpriteFrame::create("Blue_Right1.png", Rect(0,0,65,81)));
animFrames.pushBack(SpriteFrame::create("Blue_Right2.png", Rect(0,0,65,81)));
animFrames.pushBack(SpriteFrame::create("Blue_Right3.png", Rect(0,0,65,81)));

// create the animation out of the frames
Animation* animation = Animation::createWithSpriteFrames(animFrames, 0.1f);
Animate* animate = Animate::create(animation);

// run it and repeat it forever
mySprite->runAction(RepeatForever::create(animate));
```

## 变速运动[](https://docs.cocos.com/cocos2d-x/manual/zh/actions/basic.html#%E5%8F%98%E9%80%9F%E8%BF%90%E5%8A%A8)

变速动作可以让节点对象具有加速度，产生平滑同时相对复杂的动作，所以可以用变速动作来模仿一些物理运动，这样比实际使用物理引擎的性能消耗低，使用起来也简单。当然你也可以将变速动作应用到动画菜单和按钮上，实现你想要的效果。

![](https://docs.cocos.com/cocos2d-x/manual/en/actions/actions-img/easing-functions.png)

Cocos2d-x 支持上图中的大部分变速动作，实现起来也很简单。我们来看个例子，一个精灵从屏幕顶部落下然后不断跳动：

```
// create a sprite
auto mySprite = Sprite::create("mysprite.png");

// create a MoveBy Action to where we want the sprite to drop from.
auto move = MoveBy::create(2, Vec2(200, dirs->getVisibleSize().height -
 newSprite2->getContentSize().height));

// create a BounceIn Ease Action
auto move_ease_in = EaseBounceIn::create(move->clone() );
auto move_ease_in_back = move_ease_in->reverse();

// create a delay that is run in between sequence events
auto delay = DelayTime::create(0.25f);

// create the sequence of actions, in the order we want to run them
auto seq1 = Sequence::create(move_ease_in, delay, move_ease_in_back,
    delay->clone(), nullptr);

// run the sequence and repeat forever.
mySprite->runAction(RepeatForever::create(seq1));
```

复杂的动作很难在这样的文本里表示，要是看效果的话最好去运行一下本指南的 [代码示例](https://github.com/chukong/programmers-guide-samples/tree/v3.16)，或者运行引擎代码的测试项目 `cpp-tests`，在子菜单 `3:Actions - Basic` 中有基本的动作效果展示。

# 序列

**动作序列(Sequence)** 是一种封装多个动作的对象，当这个对象执行时被封装的动作会顺序执行。

一个 `Sequence` 可以包含任何数量的动作对象，回调方法和其它序列。可以包含回调方法? 没错! Cocos2d-x 允许把一个方法添加进去 `CallFunc` 对象，然后将 `CallFunc` 添加到 `Sequence`，这样，在执行序列的时候就能触发方法调用。因此，你能在一个序列中添加一些个性化的功能，而不仅仅是添加 Cocos2d-x 提供的有限动作。下面是一个序列的动作执行示意图：

![](https://docs.cocos.com/cocos2d-x/manual/en/actions/actions-img/sequence.png)

## Sequence 示例[](https://docs.cocos.com/cocos2d-x/manual/zh/actions/sequences.html#sequence-%E7%A4%BA%E4%BE%8B)

```
auto mySprite = Sprite::create("mysprite.png");

// create a few actions.
auto jump = JumpBy::create(0.5, Vec2(0, 0), 100, 1);

auto rotate = RotateTo::create(2.0f, 10);

// create a few callbacks
auto callbackJump = CallFunc::create([](){
    log("Jumped!");
});

auto callbackRotate = CallFunc::create([](){
    log("Rotated!");
});

// create a sequence with the actions and callbacks
auto seq = Sequence::create(jump, callbackJump, rotate, callbackRotate, nullptr);

// run it
mySprite->runAction(seq);
```

上面这个 `Sequence` 做了什么? 按照下面的顺序执行了每一个动作。

**Jump** -> **callbackJump()** -> **Rotate** -> **callbackRotate()**

## Spawn[](https://docs.cocos.com/cocos2d-x/manual/zh/actions/sequences.html#spawn)

`Spawn` 和 `Sequence` 是非常相似的，区别是 `Spawn` 同时执行所有的动作。`Spawn` 对象可以添加任意数量的动作和其它 `Spawn` 对象。

![](https://docs.cocos.com/cocos2d-x/manual/en/actions/actions-img/spawn.png)

`Spawn` 的效果和同时运行多个动作的 `runAction()` 方法是一致的，但是它的独特之处是 `Spawn` 能被放到 `Sequence` 中，结合 `Spawn` 和 `Sequence` 能实现非常强大的动作效果。

例如，创建两个动作：

```
// create 2 actions and run a Spawn on a Sprite
auto mySprite = Sprite::create("mysprite.png");

auto moveBy = MoveBy::create(10, Vec2(400,100));
auto fadeTo = FadeTo::create(2.0f, 120.0f);
```

使用 `Spawn`：

```
// running the above Actions with Spawn.
auto mySpawn = Spawn::createWithTwoActions(moveBy, fadeTo);
mySprite->runAction(mySpawn);
```

同时调用方法 `runAction()`：

```
// running the above Actions with consecutive runAction() statements。
mySprite->runAction(moveBy);
mySprite->runAction(fadeTo);
```

上面两种方式产生的效果是一样的，现在看把一个 `Spawn` 添加到一个 `Sequence` 中是怎样的一种情景，动作的执行流程会看起来像这样：

![](https://docs.cocos.com/cocos2d-x/manual/en/actions/actions-img/spawn_in_a_sequence.png)

```
// create a Sprite
auto mySprite = Sprite::create("mysprite.png");

// create a few Actions
auto moveBy = MoveBy::create(10, Vec2(400,100));
auto fadeTo = FadeTo::create(2.0f, 120.0f);
auto scaleBy = ScaleBy::create(2.0f, 3.0f);

// create a Spawn to use
auto mySpawn = Spawn::createWithTwoActions(scaleBy, fadeTo);

// tie everything together in a sequence
auto seq = Sequence::create(moveBy, mySpawn, moveBy, nullptr);

// run it
mySprite->runAction(seq);
```

运行本文档的 [代码示例](https://github.com/chukong/programmers-guide-samples/tree/v3.16) 去看一下效果吧!

# 动作的克隆

**克隆(Clone)** 的功能和字面含义一样，如果你对一个节点对象使用了 `clone()` 方法，你就获得了这个节点对象的拷贝。

为什么要使用 `clone()` 方法? 因为当 `Action` 对象运行时会产生一个内部状态，记录着节点属性的改变。当你想将一个创建的动作，重复使用到不同的节点对象时，如果不用 `clone()` 方法，就无法确定这个动作的属性到底是怎样的(因为被使用过，产生了内部状态)，这会造成难以预料的结果。

我们来看示例，假如你有一个坐标位置是 `(0,0)` 的 `heroSprite`，执行这样一个动作：

```
MoveBy::create(10, Vec2(400,100));
```

你的 `heroSprite` 就在 10s 的时间中，从 `(0,0)` 移动到了 `(400,100)`，`heroSprite` 有了一个新位置 `(400,100)`，更重要的是动作对象也有了节点位置相关的内部状态了。现在假如你有一个坐标位置是 `(200,200)`的 `emenySprite`。你还使用这个相同的动作，`emenySprite` 就会移动到 `(800,200)`的坐标位置，并不是你期待的结果。因为第二次将这个动作应用的时候，它已经有内部状态了。使用 `clone()` 能避免这种情况，克隆获得一个新的动作对象，新的对象没有之前的内部状态。

从代码中学习用法吧，先看看错误的情况：

```
// create our Sprites
auto heroSprite = Sprite::create("herosprite.png");
auto enemySprite = Sprite::create("enemysprite.png");

// create an Action
auto moveBy = MoveBy::create(10, Vec2(400,100));

// run it on our hero
heroSprite->runAction(moveBy);

// run it on our enemy
enemySprite->runAction(moveBy); // oops, this will not be unique!
// uses the Actions current internal state as a starting point.
```

使用 `clone()` 的正确情况：

```
// create our Sprites
auto heroSprite = Sprite::create("herosprite.png");
auto enemySprite = Sprite::create("enemysprite.png");

// create an Action
auto moveBy = MoveBy::create(10, Vec2(400,100));

// run it on our hero
heroSprite->runAction(moveBy);

// run it on our enemy
enemySprite->runAction(moveBy->clone()); // correct! This will be unique
```

## 动作的倒转[](https://docs.cocos.com/cocos2d-x/manual/zh/actions/sequence_internals.html#%E5%8A%A8%E4%BD%9C%E7%9A%84%E5%80%92%E8%BD%AC)

**倒转(Reverse)** 的功能也和字面意思一样，调用 `reverse()` 可以让一系列动作按相反的方向执行。`reverse()` 不是只能简单的让一个 `Action` 对象反向执行，还能让 `Sequence` 和 `Spawn` 倒转。

倒转使用起来很简单：

```
// reverse a sequence, spawn or action
mySprite->runAction(mySpawn->reverse());
```

思考下面这段代码在执行的时候, 内部发生了什么?

```
// create a Sprite
auto mySprite = Sprite::create("mysprite.png");
mySprite->setPosition(50, 56);

// create a few Actions
auto moveBy = MoveBy::create(2.0f, Vec2(500,0));
auto scaleBy = ScaleBy::create(2.0f, 2.0f);
auto delay = DelayTime::create(2.0f);

// create a sequence
auto delaySequence = Sequence::create(delay, delay->clone(), delay->clone(),
delay->clone(), nullptr);

auto sequence = Sequence::create(moveBy, delay, scaleBy, delaySequence, nullptr);

// run it
mySprite->runAction(sequence);

// reverse it
mySprite->runAction(sequence->reverse());
```

思考起来可能有点困难，我们将执行的每一步列出来，或许能帮助你理解：

1. `mySprite` 创建
2. `mySprite` 的坐标位置设置成(50,56)
3. `sequence` 开始执行
4. `sequence` 执行第一个动作 `moveBy`，2s 中 `mySprite` 移动到了坐标位置(550,56)
5. `sequence` 执行第二个动作， 暂停 2s
6. `sequence` 执行第三个动作，`scaleBy`，2s 中 `mySprite` 放大了2倍
7. `sequence` 执行第四个动作，`delaySequence`，暂停 6s
8. `reverse()` 被调用，序列倒转，开始反向执行
9. `sequence` 执行第四个动作，`delaySequence`，暂停 6s
10. `sequence` 执行第三个动作，`scaleBy`，2s 中 `mySprite` 缩小了2倍 (注意：序列内的动作被倒转)
11. `sequence` 执行第二个动作， 暂停 2s
12. `sequence` 执行第一个动作 `moveBy`，2s 中 `mySprite` 从坐标位置 (550,56)，移动到了 (50, 56)
13. `mySprite` 回到了最初的位置

我们能发现 `reverse()` 方法使用起来很简单，内部逻辑却一点都不简单。因为 Cocos2d-x 封装了复杂的逻辑，为你留下了简单易用的接口！

# 场景(Scene)

**场景(Scene)** 是一个容器，容纳游戏中的各个元素，如精灵，标签，节点对象。它负责着游戏的运行逻辑，以帧为单位渲染内容。

可以想象游戏就像一个电影，场景是观看者能看到的正在发生的情景。一个电影至少需要一个场景，一个游戏也至少需要一个 `Scene`。在使用 Cocos2d-x 进行游戏的开发中，你可以制作任意数量的场景，并在不同场景间轻松切换。

# 场景创建

创建一个场景非常简单：

```
auto myScene = Scene::create();
```

## 还记得场景图吗[](https://docs.cocos.com/cocos2d-x/manual/zh/scenes/creating.html#%E8%BF%98%E8%AE%B0%E5%BE%97%E5%9C%BA%E6%99%AF%E5%9B%BE%E5%90%97)

第二章中我们学到了 [场景图(Scene Graph)](https://docs.cocos.com/cocos2d-x/manual/zh/basic_concepts/scene.html) 以及在游戏中它是如何生效的。要记得场景图决定了场景内节点对象的渲染顺序，也要记得 **z-order** 是如何影响场景图的。

_渲染时 `z-order` 值大的节点对象会后绘制，值小的节点对象先绘制_

## 一个简单场景[](https://docs.cocos.com/cocos2d-x/manual/zh/scenes/creating.html#%E4%B8%80%E4%B8%AA%E7%AE%80%E5%8D%95%E5%9C%BA%E6%99%AF)

让我们构建一个简单的场景，来学习场景的使用。记得 Cocos2d-x 用右手坐标系，也就是说坐标原点(0,0)在展示区的左下角，当你在场景里放置一些节点对象设置坐标位置时，注意左下角是坐标计算的起点。

```
auto dirs = Director::getInstance();
Size visibleSize = dirs->getVisibleSize();

auto myScene = Scene::create();

auto label1 = Label::createWithTTF("My Game", "Marker Felt.ttf", 36);
label1->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));

myScene->addChild(label1);

auto sprite1 = Sprite::create("mysprite.png");
sprite1->setPosition(Vec2(100, 100));

myScene->addChild(sprite1);
```

当运行这个代码的时候，我们会看到有一个场景，场景里面有一个标签和一个精灵。这虽然很简单，但这却是开发一个游戏最重要的开始！

# 场景切换

开始一个新游戏，改变关卡，或结束游戏时，为了给用户不同的效果呈现，大多需要切换不同的场景。Cocos2d-x 提供了一系列方式去做这件事情 **场景切换**。

## 场景切换的方式[](https://docs.cocos.com/cocos2d-x/manual/zh/scenes/transitioning.html#%E5%9C%BA%E6%99%AF%E5%88%87%E6%8D%A2%E7%9A%84%E6%96%B9%E5%BC%8F)

有很多场景切换的方式，每种都有特定的方法，让我们来看看：

```
auto myScene = Scene::create();
```

**`runWithScene()`** 用于开始游戏，加载第一个场景。只用于第一个场景！

```
Director::getInstance()->runWithScene(myScene);
```

**`replaceScene()`** 使用传入的场景替换当前场景来切换画面，当前场景被释放。这是切换场景时最常用的方法。

```
Director::getInstance()->replaceScene(myScene);
```

**`pushScene()`** 将当前运行中的场景暂停并压入到场景栈中，再将传入的场景设置为当前运行场景。只有存在正在运行的场景时才能调用该方法。

```
Director::getInstance()->pushScene(myScene);
```

**`popScene()`** 释放当前场景，再从场景栈中弹出栈顶的场景，并将其设置为当前运行场景。如果栈为空，直接结束应用。

```
Director::getInstance()->popScene();
```

## 场景切换的效果设置[](https://docs.cocos.com/cocos2d-x/manual/zh/scenes/transitioning.html#%E5%9C%BA%E6%99%AF%E5%88%87%E6%8D%A2%E7%9A%84%E6%95%88%E6%9E%9C%E8%AE%BE%E7%BD%AE)

在场景切换的过程中，你可以添加一些效果：

```
auto myScene = Scene::create();

// Transition Fade
Director::getInstance()->replaceScene(TransitionFade::create(0.5, myScene, Color3B(0,255,255)));

// FlipX
Director::getInstance()->replaceScene(TransitionFlipX::create(2, myScene));

// Transition Slide In
Director::getInstance()->replaceScene(TransitionSlideInT::create(1, myScene) );
```

# UI 组件

UI 组件不是游戏专用的，是个应用程序都可能会用几个。看一看你常使用的应用程序，肯定能发现它有使用 UI 组件。UI 代表什么，UI 组件是做什么的？

UI 代表用户界面，是 _User Interface_ 的缩写，你看到的屏幕上的东西就是用户界面。界面组件有标签，按钮，菜单，滑动条等。Cocos2d-x 提供了一套易用的 UI 组件，游戏开发过程中，你能很容易的把它们添加到游戏中。

听起来这可能很简单，但创建像 `标签(Label)` 这样的核心类实际要考虑很多问题。可以想象创建一套自定义的组件是多么的困难！当然你根本没必要这样做，因为你需要的我们都考虑到了。

# 标签(Label)

Cocos2d-x 提供 **`Label`** 对象给用户，可以使用位图字体，TrueType 字体，系统字体创建标签。这个单一的类能处理你所有的标签需求。下面介绍使用各种字体，创建标签的方法。

## BMFont[](https://docs.cocos.com/cocos2d-x/manual/zh/ui_components/labels.html#bmfont)

`BMFont` 是一个使用位图字体创建的标签类型，位图字体中的字符由点阵组成。使用这种字体标签性能非常好，但是不适合缩放。由于点阵的原因，缩放会导致失真。标签中的每一个字符都是一个单独的 `Sprite`，也就是说精灵的属性(旋转，缩放，着色等)控制都适用于这里的每个字符。

创建 `BMFont` 标签需要两个文件：`.fnt` 文件和 `.png` 文件。可以使用像 Glyph Designer 这样的工具来创建位图字体，这些文件将会自动生成。

使用位图字体创建标签：

```
auto myLabel = Label::createWithBMFont("bitmapRed.fnt", "Your Text");
```

![](https://docs.cocos.com/cocos2d-x/manual/en/ui_components/ui_components-img/LabelBMFont.png)

所有在标签中出现的字符都应该能在提供的 `.fnt` 文件找到，如果找不到，字符就不会被渲染。如果你渲染了一个 `Label`，同时它有丢失字符，记得去查看一下 `.fnt` 文件是否完备。

## TTF[](https://docs.cocos.com/cocos2d-x/manual/zh/ui_components/labels.html#ttf)

_TrueType 字体_ 和我们上面了解的位图字体不同，使用这种字体很方便，你不需要为每种尺寸和颜色单独使用字体文件。不像 BMFont，如果想不失真的缩放，就要提供多种字体文件。

要创建这种标签，需要指定 `.ttf` 字体文件名，文本字符串和字体大小。

使用 TrueType 字体创建标签：

```
auto myLabel = Label::createWithTTF("Your Text", "Marker Felt.ttf", 24);
```

![](https://docs.cocos.com/cocos2d-x/manual/en/ui_components/ui_components-img/LabelTTF.png)

虽然使用 TrueType 字体比使用位图字体更灵活，但是它渲染速度较慢，并且更改标签的属性(字体，大小)是一项非常消耗性能的操作。

如果您需要具有相同属性的多个 Label 对象，那可以创建一个 `TTFConfig` 对象来统一配置，`TTFConfig` 对象允许你设置所有标签的共同属性。

通过以下方式创建一个 `TTFConfig` 对象：

```
// create a TTFConfig files for labels to share
TTFConfig labelConfig;
labelConfig.fontFilePath = "myFont.ttf";
labelConfig.fontSize = 16;
labelConfig.glyphs = GlyphCollection::DYNAMIC;
labelConfig.outlineSize = 0;
labelConfig.customGlyphs = nullptr;
labelConfig.distanceFieldEnabled = false;

// create a TTF Label from the TTFConfig file.
auto myLabel = Label::createWithTTF(labelConfig, "My Label Text");
```

![](https://docs.cocos.com/cocos2d-x/manual/en/ui_components/ui_components-img/LabelTTFWithConfig.png)

`TTFConfig` 也能用于展示中文，日文，韩文的字符。

## SystemFont[](https://docs.cocos.com/cocos2d-x/manual/zh/ui_components/labels.html#systemfont)

`SystemFont` 是一个使用系统默认字体，默认字体大小的标签类型，这样的标签不要改变他的属性，它会使用系统的规则。

使用系统字体创建标签：

```
auto myLabel = Label::createWithSystemFont("My Label Text", "Arial", 16);
```

![](https://docs.cocos.com/cocos2d-x/manual/en/ui_components/ui_components-img/LabelWithSystemFont.png)

## 标签效果[](https://docs.cocos.com/cocos2d-x/manual/zh/ui_components/labels.html#%E6%A0%87%E7%AD%BE%E6%95%88%E6%9E%9C)

在屏幕上有标签后，它们可能看起来很普通，这时你希望让他们变漂亮。你不用创建自定义字体! Label 对象就可以对标签应用效果，包括阴影，描边，发光。

阴影效果：

```
auto myLabel = Label::createWithTTF("myFont.ttf", "My Label Text", 16);

// shadow effect is supported by all Label types
myLabel->enableShadow();
```

![](https://docs.cocos.com/cocos2d-x/manual/en/ui_components/ui_components-img/LabelWithShadow.png)

描边效果:

```
auto myLabel = Label::createWithTTF("myFont.ttf", "My Label Text", 16);

// outline effect is TTF only, specify the outline color desired
myLabel->enableOutline(Color4B::WHITE, 1));
```

![](https://docs.cocos.com/cocos2d-x/manual/en/ui_components/ui_components-img/LabelWithOutline.png)

发光效果:

```
auto myLabel = Label::createWithTTF("myFont.ttf", "My Label Text", 16);

// glow effect is TTF only, specify the glow color desired.
myLabel->enableGlow(Color4B::YELLOW);
```

![](https://docs.cocos.com/cocos2d-x/manual/en/ui_components/ui_components-img/LabelWithGlow.png)

# 菜单(Menu)

菜单是什么，我们肯定都很熟悉了，在每个游戏中都会有菜单。我们使用菜单浏览游戏选项，更改游戏设置。菜单通常包含开始，退出，设置，关于等项，菜单当然也可以包含子菜单。在 Cocos2d-x 提供 **`Menu`** 对象支持菜单功能，_`Menu` 对象是一种特殊的 `Node` 对象_。

创建一个菜单用于添加菜单项：

```
auto myMenu = Menu::create();
```

像我们刚才提到的一个菜单，总会有一些菜单项，比如开始，退出，设置等，没有菜单项的菜单没有存在的意义。Cocos2d-x 提供了一些方法来创建菜单项，比如使用 `Label` 对象，或是使用一张图像。菜单项一般有正常状态和选择状态。菜单项显示时是正常状态，当你点击它时变为选择状态，同时点击菜单还会触发一个回调函数。

使用图像创建菜单：

```
// creating a menu with a single item

// create a menu item by specifying images
auto closeItem = MenuItemImage::create("CloseNormal.png", "CloseSelected.png",
CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));

auto menu = Menu::create(closeItem, NULL);
this->addChild(menu, 1);
```

还可以使用 `MenuItem` 的一个 `vector` 创建菜单：

```
// creating a Menu from a Vector of items
Vector<MenuItem*> MenuItems;

auto closeItem = MenuItemImage::create("CloseNormal.png", "CloseSelected.png",
CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));

MenuItems.pushBack(closeItem);

/* repeat for as many menu items as needed */

auto menu = Menu::createWithArray(MenuItems);
this->addChild(menu, 1);
```

运行本文档的代码示例，你就能看到在 _Chapter 6_ 有一个 **Label** 菜单项组成的菜单。

![](https://docs.cocos.com/cocos2d-x/manual/en/ui_components/ui_components-img/menu.png)

## 使用 Lambda 表达式[](https://docs.cocos.com/cocos2d-x/manual/zh/ui_components/menus.html#%E4%BD%BF%E7%94%A8-lambda-%E8%A1%A8%E8%BE%BE%E5%BC%8F)

我们知道，当您点击菜单项时会触发一个回调函数。C++ 11 支持了 lambda 表达式，lambda 表达式是 [匿名函数](https://en.wikipedia.org/wiki/Anonymous_function#C.2B.2B_.28since_C.2B.2B11.29)，所以你可以在回调方法处，使用 lambda 表达式，这样能让代码看起来更简洁，同时不会有额外的性能开销。

一个简单的 lambda 表达式：

```
// create a simple Hello World lambda
auto func = [] () { cout << "Hello World"; };

// now call it someplace in code
func();
```

使用 lambda 表达式作为菜单项的回调函数：

```
auto closeItem = MenuItemImage::create("CloseNormal.png", "CloseSelected.png",
[&](Ref* sender){
    // your code here
});
```

# 按钮(Button)

按钮是什么，好像没有必要解释，我们都知道这东西是用来点击的，点击后使我们的游戏产生一些变化，比如更改了场景，触发了动作等等。按钮会拦截点击事件，事件触发时调用事先定义好的回调函数。按钮有一个正常状态，一个选择状态，还有一个不可点击状态，按钮的外观可以根据这三个状态而改变。Cocos2d-x 提供 **`Button`** 对象支持按钮功能，创建一个按钮并定义一个回调函数很简单，记得在操作的时候要有头文件包含: `#include "ui/CocosGUI.h"`。

```

auto button = Button::create("normal_image.png", "selected_image.png", "disabled_image.png");

button->setTitleText("Button Text");

button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type){
        switch (type)
        {
                case ui::Widget::TouchEventType::BEGAN:
                        break;
                case ui::Widget::TouchEventType::ENDED:
                        std::cout << "Button 1 clicked" << std::endl;
                        break;
                default:
                        break;
        }
});

this->addChild(button);
```

可以看到，我们为按钮的每个状态都指定了一个 _.png_ 图像：

![](https://docs.cocos.com/cocos2d-x/manual/en/ui_components/ui_components-img/Button_Normal.png) ![](https://docs.cocos.com/cocos2d-x/manual/en/basic_concepts/basic_concepts-img/smallSpacer.png) ![](https://docs.cocos.com/cocos2d-x/manual/en/ui_components/ui_components-img/Button_Press.png) ![](https://docs.cocos.com/cocos2d-x/manual/en/basic_concepts/basic_concepts-img/smallSpacer.png) ![](https://docs.cocos.com/cocos2d-x/manual/en/ui_components/ui_components-img/Button_Disable.png)

在屏幕显示的时候，同一个时刻只能看到一个状态，正常显示状态像这样：

![](https://docs.cocos.com/cocos2d-x/manual/en/ui_components/ui_components-img/Button_example.png)

# 复选框(CheckBox)

日常生活中复选框很常见，比如填写问卷时，让我们选一些喜欢的项目，游戏设置中，某一设置是打开还是关闭。只有两种状态的项目经常被设计为复选框。Cocos2d-x 提供 **`Checkbox`** 对象支持复选框功能。

创建一个复选框：

```
#include "ui/CocosGUI.h"

auto checkbox = CheckBox::create("check_box_normal.png",
                                 "check_box_normal_press.png",
                                 "check_box_active.png",
                                 "check_box_normal_disable.png",
                                 "check_box_active_disable.png");

checkbox->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type){
        switch (type)
        {
                case ui::Widget::TouchEventType::BEGAN:
                        break;
                case ui::Widget::TouchEventType::ENDED:
                        std::cout << "checkbox 1 clicked" << std::endl;
                        break;
                default:
                        break;
        }
});

this->addChild(checkbox);
```

在上面的例子中，我们能看到为一个复选框指定了五张图像，因为复选框有五种状态: 未被选中，被点击，未被选中时不可用，被选中，选中时不可用。这样五种状态的图像依次如下：

![](https://docs.cocos.com/cocos2d-x/manual/en/ui_components/ui_components-img/CheckBox_Normal.png) ![](https://docs.cocos.com/cocos2d-x/manual/en/basic_concepts/basic_concepts-img/smallSpacer.png) ![](https://docs.cocos.com/cocos2d-x/manual/en/ui_components/ui_components-img/CheckBox_Press.png) ![](https://docs.cocos.com/cocos2d-x/manual/en/basic_concepts/basic_concepts-img/smallSpacer.png) ![](https://docs.cocos.com/cocos2d-x/manual/en/ui_components/ui_components-img/CheckBox_Disable.png) ![](https://docs.cocos.com/cocos2d-x/manual/en/basic_concepts/basic_concepts-img/smallSpacer.png) ![](https://docs.cocos.com/cocos2d-x/manual/en/ui_components/ui_components-img/CheckBoxNode_Normal.png) ![](https://docs.cocos.com/cocos2d-x/manual/en/basic_concepts/basic_concepts-img/smallSpacer.png) ![](https://docs.cocos.com/cocos2d-x/manual/en/ui_components/ui_components-img/CheckBoxNode_Disable.png)

在屏幕显示的时候，同一个时刻只能看到一个状态，被选中时状态像这样：

![](https://docs.cocos.com/cocos2d-x/manual/en/ui_components/ui_components-img/Checkbox_example.png)

# 进度条(LoadingBar)

如果你经常玩游戏，那肯定见过一个情景：屏幕上显示了一个进度条，提示资源正在加载中，这个条表示资源加载的进度。Cocos2d-x 提供 **`LoadingBar`** 对象支持进度条。

创建一个进度条：

```
#include "ui/CocosGUI.h"

auto loadingBar = LoadingBar::create("LoadingBarFile.png");

// set the direction of the loading bars progress
loadingBar->setDirection(LoadingBar::Direction::RIGHT);

this->addChild(loadingBar);
```

上面的例子，我们创建了一个进度条，设置了当进度增加时，进度条向右填充。

在进度的控制中，你肯定需要改变进度条的进度. 示例：

```
#include "ui/CocosGUI.h"

auto loadingBar = LoadingBar::create("LoadingBarFile.png");
loadingBar->setDirection(LoadingBar::Direction::RIGHT);

// something happened, change the percentage of the loading bar
loadingBar->setPercent(25);

// more things happened, change the percentage again.
loadingBar->setPercent(35);

this->addChild(loadingBar);
```

上面例子，使用的进度条图像是：

![](https://docs.cocos.com/cocos2d-x/manual/en/ui_components/ui_components-img/LoadingBarFile.png)

在屏幕上一个满进度的进度条是这样的：

![](https://docs.cocos.com/cocos2d-x/manual/en/ui_components/ui_components-img/LoadingBar_example.png)

# 滑动条(Slider)

有时候你想平滑的改变一个值，比如游戏设置中，调整背景音乐的音量，或着你有一个角色，允许用户设置攻击敌人的力量。这种场景最适合使用滑动条，Cocos2d-x 提供 **`Slider`** 对象支持滑动条。

创建滑动条：

```
#include "ui/CocosGUI.h"

auto slider = Slider::create();
slider->loadBarTexture("Slider_Back.png"); // what the slider looks like
slider->loadSlidBallTextures("SliderNode_Normal.png", "SliderNode_Press.png", "SliderNode_Disable.png");
slider->loadProgressBarTexture("Slider_PressBar.png");

slider->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type){
        switch (type)
        {
                case ui::Widget::TouchEventType::BEGAN:
                        break;
                case ui::Widget::TouchEventType::ENDED:
                        std::cout << "slider moved" << std::endl;
                        break;
                default:
                        break;
        }
});

this->addChild(slider);
```

从上面的例子，可以看出，实现一个滑动条需要提供五张图像，对应滑动条的不同部分不同状态，分别为：滑动条背景，上层进度条，正常显示时的滑动端点，滑动时的滑动端点，不可用时的滑动端点。本次示例的五张图像如下：

![](https://docs.cocos.com/cocos2d-x/manual/en/ui_components/ui_components-img/Slider_Back.png) ![](https://docs.cocos.com/cocos2d-x/manual/en/basic_concepts/basic_concepts-img/smallSpacer.png) ![](https://docs.cocos.com/cocos2d-x/manual/en/ui_components/ui_components-img/Slider_PressBar.png) ![](https://docs.cocos.com/cocos2d-x/manual/en/basic_concepts/basic_concepts-img/smallSpacer.png) ![](https://docs.cocos.com/cocos2d-x/manual/en/ui_components/ui_components-img/SliderNode_Normal.png) ![](https://docs.cocos.com/cocos2d-x/manual/en/basic_concepts/basic_concepts-img/smallSpacer.png) ![](https://docs.cocos.com/cocos2d-x/manual/en/ui_components/ui_components-img/SliderNode_Press.png) ![](https://docs.cocos.com/cocos2d-x/manual/en/basic_concepts/basic_concepts-img/smallSpacer.png) ![](https://docs.cocos.com/cocos2d-x/manual/en/ui_components/ui_components-img/SliderNode_Disable.png)

在屏幕上一个滑动条看起来是这样的：

![](https://docs.cocos.com/cocos2d-x/manual/en/ui_components/ui_components-img/Slider_example.png)

# 文本框(TextField)

如果你想让参与游戏的玩家可以自定义一个昵称怎么办，在哪里输入文本？Cocos2d-x 提供 **`TextField`** 满足这种需求。它支持触摸事件，焦点，定位内容百分比等。

创建一个文本框：

```
#include "ui/CocosGUI.h"

auto textField = TextField::create("","Arial",30);

textField->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type){
                std::cout << "editing a TextField" << std::endl;
});

this->addChild(textField);
```

这个例子中，创建了一个 `TextField`，指定了回调函数。

提供的文本框对象，是多功能的，能满足所有的输入需求，比如用户密码的输入，限制用户可以输入的字符数等等！

看一个例子：

```
#include "ui/CocosGUI.h"

auto textField = TextField::create("","Arial",30);

// make this TextField password enabled
textField->setPasswordEnabled(true);

// set the maximum number of characters the user can enter for this TextField
textField->setMaxLength(10);

textField->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type){
                std::cout << "editing a TextField" << std::endl;
});

this->addChild(textField);
```

屏幕上一个文本框是这样的：

![](https://docs.cocos.com/cocos2d-x/manual/en/ui_components/ui_components-img/TextField_example.png)

当点击文本框，键盘就会自动调出来，此时可以输入文本：

![](https://docs.cocos.com/cocos2d-x/manual/en/ui_components/ui_components-img/TextField_example_keyboard.png)

# 高级节点对象

除了 `Label`，`Sprite` 这些基本的节点对象外，Cocos2d-x 还提供了一些特殊的节点对象，来帮助构建一些高级功能。

也许你想制作一个基于瓦片地图的游戏，也许你想添加粒子效果，也许你想在游戏中添加一个 2D 滚动的边栏，别担心，这些特殊的节点对象能帮助你。

# 瓦片地图

在游戏开发过程中，我们会遇到超过屏幕大小的地图，例如在即时战略游戏中，它使得玩家可以在地图中滚动游戏画面。这类游戏通常会有丰富的背景元素，如果直接使用背景图切换的方式，需要为每个不同的场景准备一张背景图，而且每个背景图都不小，这样会造成资源浪费。

瓦片地图就是为了解决这问题而产生的。一张大的世界地图或者背景图可以由几种地形来表示，每种地形对应一张小的的图片，我们称这些小的地形图片为瓦片。把这些瓦片拼接在一起，一个完整的地图就组合出来了，这就是瓦片地图的原理。

在 Cocos2d-x 中，瓦片地图实现的是 TileMap 方案，TileMap 要求每个瓦片占据地图上一个四边形或六边形的区域。把不同的瓦片拼接在一起，就可以组成完整的地图。TileMap 使用一种基于 XML 的 TMX 格式文件。

使用 TMX 文件创建一个瓦片地图：

```
// reading in a tiled map.
auto map = TMXTiledMap::create("TileMap.tmx");
addChild(map, 0, 99); // with a tag of '99'
```

瓦片地图可能有许多层，通过层名获取到一个特定的层。

```
// how to get a specific layer
auto map = TMXTiledMap::create("TileMap.tmx");
auto layer = map->getLayer("Layer0");
auto tile = layer->getTileAt(Vec2(1, 63));
```

每个瓦片都有独一无二的位置和 ID，这使得我们很容易选择特定的瓦片。

通过位置访问：

```
// to obtain a specific tiles id
unsigned int gid = layer->getTileGIDAt(Vec2(0, 63));
```

瓦片地图布局示例：

![](https://docs.cocos.com/cocos2d-x/manual/en/other_node_types/other_node_types-img/tilemap1.png "timemap1")

![](https://docs.cocos.com/cocos2d-x/manual/en/other_node_types/other_node_types-img/tilemap2.png "timemap2")

有很多工具可以用来制作瓦片地图，[Tiled](http://mapeditor.org/) 就是其中一款流行的制作工具，它有一个活跃的用户社区。推荐你去使用，上面的屏幕截图就来自 [Tiled](http://mapeditor.org/) 的项目。

# 粒子系统

粒子系统是指计算机图形学中模拟特定现象的技术，它在模仿自然现象、物理现象及空间扭曲上具备得天独厚的优势，能为我们实现一些真实自然而又带有随机性的效果（如爆炸、烟花、水流）提供了方便。Cocos2d-x引擎中就为我们提供了强大的粒子系统。

下面是使用粒子系统完成的两个粒子特效：

![](https://docs.cocos.com/cocos2d-x/manual/en/other_node_types/other_node_types-img/particle1.png "snow") ![](https://docs.cocos.com/cocos2d-x/manual/en/basic_concepts/basic_concepts-img/smallSpacer.png) ![](https://docs.cocos.com/cocos2d-x/manual/en/other_node_types/other_node_types-img/particle3.png "sun")

## 创建粒子特效的工具[](https://docs.cocos.com/cocos2d-x/manual/zh/other_node_types/particles.html#%E5%88%9B%E5%BB%BA%E7%B2%92%E5%AD%90%E7%89%B9%E6%95%88%E7%9A%84%E5%B7%A5%E5%85%B7)

尽管你能手动创建粒子特效，按照喜好确定每个属性，但是使用工具往往更方便高效。下面介绍几个第三方工具：

1. [Particle Designer](https://71squared.com/particledesigner)：Mac 上一款非常强大的粒子特效编辑器
2. [V-play particle editor](http://v-play.net/2014/02/v-play-particle-editor-for-cocos2d-and-v-play/)：一款跨平台的粒子特效编辑器
3. [Particle2dx](http://www.effecthub.com/particle2dx)：一款 Web 粒子特效编辑器，打开网页即可进行设计

使用这些工具完成粒子特效的设计，最终会导出一个 _.plist_ 文件，Cocos2d-x 通过使用这种文件，就能把粒子特效添加到场景中，添加方法和操作一个普通的节点类型一样。

创建方法：

```
// create by plist file
auto particleSystem = ParticleSystem::create("SpinningPeas.plist");
```

## 内置粒子特效[](https://docs.cocos.com/cocos2d-x/manual/zh/other_node_types/particles.html#%E5%86%85%E7%BD%AE%E7%B2%92%E5%AD%90%E7%89%B9%E6%95%88)

准备好添加粒子特效到你的游戏中了吗？是否习惯创建自定义粒子特效？不习惯也没关系，我们内置了一些粒子特效，你可以直接使用。这个列表都是：

> - ParticleFire: Point particle system. Uses Gravity mode.
> - ParticleFireworks: Point particle system. Uses Gravity mode.
> - ParticleSun: Point particle system. Uses Gravity mode.
> - ParticleGalaxy: Point particle system. Uses Gravity mode.
> - ParticleFlower: Point particle system. Uses Gravity mode.
> - ParticleMeteor: Point particle system. Uses Gravity mode.
> - ParticleSpiral: Point particle system. Uses Gravity mode.
> - ParticleExplosion: Point particle system. Uses Gravity mode.
> - ParticleSmoke: Point particle system. Uses Gravity mode.
> - ParticleSnow: Point particle system. Uses Gravity mode.
> - ParticleRain: Point particle system. Uses Gravity mode.

比如使用内置的烟火特效 `ParticleFireworks`：

```
auto emitter = ParticleFireworks::create();

addChild(emitter, 10);
```

是这样的效果:

![](https://docs.cocos.com/cocos2d-x/manual/en/other_node_types/other_node_types-img/particle2.png "particle fireworks")

要是内置的粒子特效不是你想要的那样，也没关系，你可以直接手动设置参数！让我们拿上面的烟火特效示例，并通过手动改变属性进一步控制。

```
auto emitter = ParticleFireworks::create();

// set the duration
emitter->setDuration(ParticleSystem::DURATION_INFINITY);

// radius mode
emitter->setEmitterMode(ParticleSystem::Mode::RADIUS);

// radius mode: 100 pixels from center
emitter->setStartRadius(100);
emitter->setStartRadiusVar(0);
emitter->setEndRadius(ParticleSystem::START_RADIUS_EQUAL_TO_END_RADIUS);
emitter->setEndRadiusVar(0);    // not used when start == end

addChild(emitter, 10);
```

# 视差滚动

视差滚动是指让多层背景以不同的速度移动，从而形成的立体运动效果。比如超级马里奥游戏中，角色所在地面的移动与背景天空的移动，就是一个视差滚动。Cocos2d-x 通过 **`ParallaxNode`** 对象模拟视差滚动。可以通过序列控制移动，也可以通过监听鼠标，触摸，加速度计，键盘等事件控制移动。`ParallaxNode` 对象比常规节点对象复杂一些，因为为了呈现不同的移动速度，需要多个子节点。它类似 `Menu` 像一个容器，本身不移动，移动的是被添加进入其中的不同子节点。`ParallaxNode` 的创建：

```
// create ParallaxNode
auto paraNode = ParallaxNode::create();
```

添加多个节点对象：

```
// create ParallaxNode
auto paraNode = ParallaxNode::create();

// background image is moved at a ratio of 0.4x, 0.5y
paraNode->addChild(background, -1, Vec2(0.4f,0.5f), Vec2::ZERO);

// tiles are moved at a ratio of 2.2x, 1.0y
paraNode->addChild(middle_layer, 1, Vec2(2.2f,1.0f), Vec2(0,-200) );

// top image is moved at a ratio of 3.0x, 2.5y
paraNode->addChild(top_layer, 2, Vec2(3.0f,2.5f), Vec2(200,800) );
```

需要注意的是，被添加的每个 Node 对象被赋予了一个唯一的 `z-order` 顺序，以便他们堆叠在彼此之上。另外要注意 `addChild()` 调用中两个 `Vec2` 参数，第一个决定这个子节点的移动速度与父节点移动速度的比率，第二个是相对父节点 `ParallaxNode` 的偏移量。

在文本中很难展示视差滚动，请运行本文档的代码示例吧！

# 事件分发机制

Cocos2d-x 通过事件分发机制响应用户事件，已内置支持常见的事件如触摸事件，键盘事件等。同时提供了创建自定义事件的方法，满足我们在游戏的开发过程中，特殊的事件响应需求。

## 基本元素[](https://docs.cocos.com/cocos2d-x/manual/zh/event_dispatcher/#%E5%9F%BA%E6%9C%AC%E5%85%83%E7%B4%A0)

- 事件监听器：负责接收事件，并执行预定义的事件处理函数
- 事件分发器：负责发起通知
- 事件对象：记录事件的相关信息
# 监听器

## 五种类型[](https://docs.cocos.com/cocos2d-x/manual/zh/event_dispatcher/types.html#%E4%BA%94%E7%A7%8D%E7%B1%BB%E5%9E%8B)

- `EventListenerTouch` - 响应触摸事件
    
- `EventListenerKeyboard` - 响应键盘事件
    
- `EventListenerAcceleration` - 响应加速度事件
    
- `EventListenMouse` - 响应鼠标事件
    
- `EventListenerCustom` - 响应自定义事件
    

## 事件的吞没[](https://docs.cocos.com/cocos2d-x/manual/zh/event_dispatcher/types.html#%E4%BA%8B%E4%BB%B6%E7%9A%84%E5%90%9E%E6%B2%A1)

当你有一个监听器，已经接收到了期望的事件，这时事件应该被吞没。事件被吞没，意味着在事件的传递过程中，你消耗了此事件，事件不再向下传递。避免了下游的其它监听器获取到此事件。

设置吞没：

```
// When "swallow touches" is true, then returning 'true' from the
// onTouchBegan method will "swallow" the touch event, preventing
// other listeners from using it.
listener1->setSwallowTouches(true);

// you should also return true in onTouchBegan()

listener1->onTouchBegan = [](Touch* touch, Event* event){
    // your code

    return true;
};
```

# 优先级

事件的吞没中，我们提到了事件的传递。事件如何传递，先到哪个监听器？这是由优先级决定的。

**固定值优先级** 使用一个整形的数值，数值较低的监听器比数值较高的监听器，先接收到事件。

**场景图优先级** 是指向节点对象的指针，`z-order` 较高的节点中的监听器比 `z-order` 较低的节点中的，先接收到事件。_由于 `z-order` 较高的节点在顶部绘制，所以使用这种优先级可以确保触摸事件被正确响应_

还记得这个场景图吗？图像绘制时，是按照 `A, B, C, D, E, F, G, H, I` 的顺序。

![](https://docs.cocos.com/cocos2d-x/manual/en/basic_concepts/basic_concepts-img/in-order-walk.png "in-order walk")

当使用 _场景图优先级_ 时，事件是按照绘制的反方向，即 `I, H, G, F, E, D, C, B, A` 传递。如果一个事件被触发，`I` 节点先接收到，如果在 `I` 节点中事件被吞没，则不会继续传递，未被吞没，事件将传递到 `H` 节点，每个节点都重复同样的逻辑，直到事件被吞没，或者传递结束，本次事件触发才完成。

# 触摸事件

触摸事件是手机游戏中最重要的事件，它易于创建，还能提供多种多样的功能。

让我们先了解一下什么是触摸事件，当你触摸移动设备的屏幕时，设备感受到被触摸，了解到被触摸的位置，同时取得触摸到的内容，然后你的触摸被回答。 这就是触摸事件。

> 如果你希望通过触摸控制屏幕下层的对象，那可以通过 [优先级](https://docs.cocos.com/cocos2d-x/manual/zh/event_dispatcher/priority.html)，达成这种需求，优先级高的对象能先处理事件。

创建触摸事件监听器：

```
//  Create a "one by one" touch event listener
// (processes one touch at a time)
auto listener1 = EventListenerTouchOneByOne::create();

// trigger when you push down
listener1->onTouchBegan = [](Touch* touch, Event* event){
    // your code
    return true; // if you are consuming it
};

// trigger when moving touch
listener1->onTouchMoved = [](Touch* touch, Event* event){
    // your code
};

// trigger when you let up
listener1->onTouchEnded = [=](Touch* touch, Event* event){
    // your code
};

// Add listener
_eventDispatcher->addEventListenerWithSceneGraphPriority(listener1, this);
```

可以看到，在使用触摸事件监听器时，可以监听三种不同的事件，每一个事件都有自己触发的时机。

三种事件及其触发时机：

- **`onTouchBegan`** 开始触摸屏幕时
- **`onTouchMoved`** 触摸屏幕，同时在屏幕上移动时
- **`onTouchEnded`** 结束触摸屏幕时
# 键盘事件

对于桌面游戏，一般需要通过键盘做一些游戏内的控制，这时你就需要监听键盘事件。Cocos2d-x 支持键盘事件，就像上节介绍的触摸事件一样。

创建键盘事件监听器：

```
// creating a keyboard event listener
auto listener = EventListenerKeyboard::create();
listener->onKeyPressed = CC_CALLBACK_2(KeyboardTest::onKeyPressed, this);
listener->onKeyReleased = CC_CALLBACK_2(KeyboardTest::onKeyReleased, this);

_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

// Implementation of the keyboard event callback function prototype
void KeyboardTest::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event)
{
        log("Key with keycode %d pressed", keyCode);
}

void KeyboardTest::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event)
{
        log("Key with keycode %d released", keyCode);
}
```

可以看到，在使用键盘事件监听器时，可以监听两种不同的事件，每一个事件都有自己的触发时机。

两种事件及触发时机：

- **`onKeyPressed`** 按键被按下时
- **`onKeyReleased`** 按下状态的按键被放开时
# 加速度传感器事件

现在一些移动设备配备有加速度传感器，我们可以通过监听它的事件获取各方向的加速度。

可以设想要完成一个游戏情景：通过来回移动手机，平衡小球在手机中的位置。这种场景的完成，就需要监听加速度传感器事件。

使用加速度传感器，需要先启用

```
Device::setAccelerometerEnabled(true);
```

创建加速度传感器监听器：

```
// creating an accelerometer event
auto listener = EventListenerAcceleration::create(CC_CALLBACK_2(
AccelerometerTest::onAcceleration, this));

_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

// Implementation of the accelerometer callback function prototype
void AccelerometerTest::onAcceleration(Acceleration* acc, Event* event)
{
    //  Processing logic here
}
```

# 鼠标事件

就像前几节介绍的那样，Cocos2d-x 支持响应鼠标事件

创建鼠标事件监听器：

```
_mouseListener = EventListenerMouse::create();
_mouseListener->onMouseMove = CC_CALLBACK_1(MouseTest::onMouseMove, this);
_mouseListener->onMouseUp = CC_CALLBACK_1(MouseTest::onMouseUp, this);
_mouseListener->onMouseDown = CC_CALLBACK_1(MouseTest::onMouseDown, this);
_mouseListener->onMouseScroll = CC_CALLBACK_1(MouseTest::onMouseScroll, this);

_eventDispatcher->addEventListenerWithSceneGraphPriority(_mouseListener, this);

void MouseTest::onMouseDown(Event *event)
{
    // to illustrate the event....
    EventMouse* e = (EventMouse*)event;
    string str = "Mouse Down detected, Key: ";
    str += tostr(e->getMouseButton());
}

void MouseTest::onMouseUp(Event *event)
{
    // to illustrate the event....
    EventMouse* e = (EventMouse*)event;
    string str = "Mouse Up detected, Key: ";
    str += tostr(e->getMouseButton());
}

void MouseTest::onMouseMove(Event *event)
{
    // to illustrate the event....
    EventMouse* e = (EventMouse*)event;
    string str = "MousePosition X:";
    str = str + tostr(e->getCursorX()) + " Y:" + tostr(e->getCursorY());
}

void MouseTest::onMouseScroll(Event *event)
{
    // to illustrate the event....
    EventMouse* e = (EventMouse*)event;
    string str = "Mouse Scroll detected, X: ";
    str = str + tostr(e->getScrollX()) + " Y: " + tostr(e->getScrollY());
}
```

# 自定义事件

上述提到的事件都是系统内置的，如触摸事件，键盘事件等。此外，你可以制作自定义事件，这些事件不是由系统控制触发的，而是通过代码手动触发。

创建自定义事件监听器：

```
_listener = EventListenerCustom::create("game_custom_event1", [=](EventCustom* event){
    std::string str("Custom event 1 received, ");
    char* buf = static_cast<char*>(event->getUserData());
    str += buf;
    str += " times";
    statusLabel->setString(str.c_str());
});

_eventDispatcher->addEventListenerWithSceneGraphPriority(_listener, this);
```

上面制作了一个自定义事件监听器，并预设了响应方法。下面创建自定义事件，并手动分发：

```
static int count = 0;
++count;

char* buf[10];
sprintf(buf, "%d", count);

EventCustom event("game_custom_event1");
event.setUserData(buf);

_eventDispatcher->dispatchEvent(&event);
```

示例创建了一个自定义事件( _EventCustom_ )对象，并设置了 `UserData`，然后调用 `_eventDispatcher->dispatchEvent(&event)` 进行手动事件分发。当预先定义的事件监听器，收到此事件，将会触发对应的响应函数。响应函数中可以获取到事件分发时设置的 `UserData` 完成数据处理。

> _注意: `EventCustom` 与 `EventListenerCustom` 的第一个参数事件名都是字符串 `game_custom_event1`_

# 进阶话题

## 注册事件监听[](https://docs.cocos.com/cocos2d-x/manual/zh/event_dispatcher/registering.html#%E6%B3%A8%E5%86%8C%E4%BA%8B%E4%BB%B6%E7%9B%91%E5%90%AC)

当我们需求多个节点对象有相同的事件响应时，可以创建一个事件监听器，然后通过 **`eventDispatcher`**，将其注册到多个对象。

以我们之前提到的触摸事件监听器为例：

```
// Add listener
_eventDispatcher->addEventListenerWithSceneGraphPriority(listener1,
sprite1);
```

需要注意的是，在添加到多个对象时，需要使用 **`clone()`** 方法。

```
// Add listener
_eventDispatcher->addEventListenerWithSceneGraphPriority(listener1,
sprite1);

// Add the same listener to multiple objects.
_eventDispatcher->addEventListenerWithSceneGraphPriority(listener1->clone(),
 sprite2);

_eventDispatcher->addEventListenerWithSceneGraphPriority(listener1->clone(),
 sprite3);
```

## 移除事件监听[](https://docs.cocos.com/cocos2d-x/manual/zh/event_dispatcher/registering.html#%E7%A7%BB%E9%99%A4%E4%BA%8B%E4%BB%B6%E7%9B%91%E5%90%AC)

按照下面的方法，可以将已经添加的事件监听器移除。

```
_eventDispatcher->removeEventListener(listener);
```

> _内置节点对象的事件分发机制，和我们上面讨论的一致，比如，当你点击带有菜单项的菜单时，也会分发一个事件。 同样的你也可以在内置节点对象上使用 `removeEventListener()` 移除事件监听。_

# 使用脚本

## 脚本组件[](https://docs.cocos.com/cocos2d-x/manual/zh/scripting/#%E8%84%9A%E6%9C%AC%E7%BB%84%E4%BB%B6)

脚本组件是用来扩展 C++ 节点对象的一种方式，你可以将脚本组件绑定到节点对象上，然后脚本组件就能收到 `onEnter`，`onExit` 和 `update` 事件。

脚本组件支持两种脚本语言 JavaScript 和 Lua，使用的脚本组件应该和绑定脚本的语言类型对应，比如 `ComponentJS` 用来绑定 JavaScript 脚本，`ComponentLua` 用来绑定 Lua 脚本。有了脚本组件，你就可以在 Cocos2d-x 的项目中，很方便的使用脚本进行一些控制。需要注意的是，在一个项目中不能混用脚本组件，也就是说一个项目要么只使用 JavaScript 脚本，要么只使用 Lua 脚本。

使用 Lua 脚本：

```
// create a Sprite and add a LUA component
auto player = Sprite::create("player.png");

auto luaComponent = ComponentLua::create("player.lua");
player->addComponent(luaComponent);
```

```
-- player.lua

local player = {
    onEnter = function(self)
        -- do some things in onEnter
    end,

    onExit = function(self)
        -- do some things in onExit
    end,

    update = function(self)
        -- do some things every frame
    end
}

-- it is needed to return player to let c++ nodes know it
return player
```

使用 JavaScript 脚本:

```
// create a Sprite and add a LUA component
auto player = Sprite::create("player.png");

auto jsComponent = ComponentJS::create("player.js");
player->addComponent(jsComponent);
```

```
// player.js
Player = cc.ComponentJS.extend({
    generateProjectile: function (x, y) {
        var projectile = new cc.Sprite("components/Projectile.png", cc.rect(0, 0, 20, 20));
        var scriptComponent = new cc.ComponentJS("src/ComponentTest/projectile.js");
        projectile.addComponent(scriptComponent);
        this.getOwner().getParent().addChild(projectile);

        // set position
        var winSize = cc.director.getVisibleSize();
        var visibleOrigin = cc.director.getVisibleOrigin();
        projectile.setPosition(cc.p(visibleOrigin.x + 20, visibleOrigin.y + winSize.height/2));

        // run action
        var posX = projectile.getPositionX();
        var posY = projectile.getPositionY();
        var offX = x - posX;
        var offY = y - posY;

        if (offX <= 0) {
            return;
        }

        var contentSize = projectile.getContentSize();
        var realX = visibleOrigin.x + winSize.width + contentSize.width/2;
        var ratio = offY / offX;
        var realY = (realX * ratio) + posY;
        var realDest = cc.p(realX, realY);

        var offRealX = realX - posX;
        var offRealY = realY - posY;
        var length = Math.sqrt((offRealX * offRealX) + (offRealY * offRealY));
        var velocity = 960;
        var realMoveDuration = length / velocity;

        projectile.runAction(cc.moveTo(realMoveDuration, realDest));
    },

    onEnter: function() {
        var owner = this.getOwner();
        owner.playerComponent = this;
        cc.eventManager.addListener({
            event: cc.EventListener.TOUCH_ALL_AT_ONCE,
            onTouchesEnded: function (touches, event) {
                var target = event.getCurrentTarget();
                if (target.playerComponent) {
                    var location = touches[0].getLocation();
                    target.playerComponent.generateProjectile(location.x, location.y);
                    jsb.AudioEngine.play2d("pew-pew-lei.wav");
                }
            }
        }, owner);
    }
});
```

注意，两种组件的使用上，有一个重要的区别。使用 Lua 组件，Lua 脚本最后需要返回 Lua 对象，使用 JavaScript 组件，JavaScript 脚本需要扩展 `cc.ComponentJS`。

更详细用法，请参考 Cocos2d-x 引擎的测试项目：`tests/lua-tests/src/ComponentTest` and `tests/js-tests/src/ComponentTest`。

# 简介

学过之前的那些章节，你就能做出来一款好玩的小游戏了，可是当你试图做一款复杂的游戏，那游戏需要模拟现实世界的情境，比如模拟两个物体碰撞，模拟物体受到重力，你就不知道该怎么办了。别担心，本章就介绍物理引擎，让我们来探索一下如何合理的使用物理引擎！

## 是否需要使用物理引擎[](https://docs.cocos.com/cocos2d-x/manual/zh/physics/#%E6%98%AF%E5%90%A6%E9%9C%80%E8%A6%81%E4%BD%BF%E7%94%A8%E7%89%A9%E7%90%86%E5%BC%95%E6%93%8E)

当你的需求很简单时，就不要使用物理引擎。比如只需要确定两个对象是否有碰撞，结合使用节点对象的 `update` 函数和 Rect 对象的 `containsPoint()`，`intersectsRect()` 方法可能就足够了。例如：

```
void update(float dt)
{
  auto p = touch->getLocation();
  auto rect = this->getBoundingBox();

  if(rect.containsPoint(p))
  {
      // do something, intersection
  }
}
```

这种检查交集以确定两个对象是否有碰撞的方法，只能解决非常简单的需求，无法扩展。比如你要开发一个游戏，一个场景有 100 个精灵对象，需要判断它们互相是否有碰撞，如果使用这种方式那将非常复杂，同时性能消耗还会严重影响 CPU 的使用率和游戏运行的帧率，这游戏根本没法玩。

这个时候就需要物理引擎了，在模拟物理情景上，物理引擎的扩展性好，性能的消耗也低。像刚才提到的那个情景，使用物理引擎就能很好的解决。初次了解物理引擎的话，肯定会觉得很陌生，我们来看一个简单的例子，通过例子来介绍术语，或许会容易接受一些。

```
// create a static PhysicsBody
auto physicsBody = PhysicsBody::createBox(Size(65.0f , 81.0f ), PhysicsMaterial(0.1f, 1.0f, 0.0f));
physicsBody->setDynamic(false);

// create a sprite
auto sprite = Sprite::create("whiteSprite.png");
sprite->setPosition(Vec2(400, 400));

// sprite will use physicsBody
sprite->addComponent(physicsBody);

//add contact event listener
auto contactListener = EventListenerPhysicsContact::create();
contactListener->onContactBegin = CC_CALLBACK_1(onContactBegin, this);
_eventDispatcher->addEventListenerWithSceneGraphPriority(contactListener, this);
```

虽然上面这个例子已经很简单了，但你可能还是觉得它复杂得有点吓人？别害怕，仔细的分析一下，就会发现也没那么复杂。

代码流程：

1. `PhysicsBody` 对象创建
2. `Sprite` 对象创建
3. `PhysicsBody` 对象以组件的形式被添加到 `Sprite` 对象
4. 创建监听器以响应 `onContactBegin()` 事件

保持耐心，一旦我们一步一步的去分析，慢慢的就能理解这个过程。

# 术语和概念

为了更好的理解物理引擎，需要先了解下面的一些术语，概念。

## 刚体(Bodies)[](https://docs.cocos.com/cocos2d-x/manual/zh/physics/concepts.html#%E5%88%9A%E4%BD%93bodies)

**刚体(Bodies)** 描述了抽象物体的物理属性，包括：质量、位置、旋转角度、速度和阻尼。Cocos2d-x 中用 `PhysicsBody` 对象表示刚体。当刚体和形状关联后，刚体对象才具有几何形状，未关联形状， 刚体只是一个抽象物体的物理属性集。

## 材质(Material)[](https://docs.cocos.com/cocos2d-x/manual/zh/physics/concepts.html#%E6%9D%90%E8%B4%A8material)

**材质(Material)** 描述了抽象物体的材料属性：

> - density：密度，用于计算物体的质量
> - friction：摩擦，用于模拟物体间的接触滑动
> - restitution：恢复系数，模拟物体反弹的一个系数，系数一般设为 0 到 1 之间。0 代表不反弹，1 代表完全反弹。

## 形状(Shape)[](https://docs.cocos.com/cocos2d-x/manual/zh/physics/concepts.html#%E5%BD%A2%E7%8A%B6shape)

**形状(Shape)** 描述了抽象物体的几何属性，将形状关联到刚体，刚体才具有几何形状。如果需要刚体具有复杂的形状，可以为它关联多个形状，每个形状对象都与一个 `PhysicsMaterial` 相关，并且拥有以下属性：type, area, mass, moment, offset 和 tag。其中有一些你可能还不熟悉，我们来逐一介绍：

> - type：描述了形状的类别，如圆形，矩形，多边形等
> - area：用于计算刚体的质量，密度和面积决定了刚体的质量
> - mass：刚体的质量，影响物体在给定的力下获得的加速度大小，物体在一个引力场中物体受到力的大小
> - moment：刚体获得特定角加速度所需要的扭矩
> - offset：在刚体的当前坐标中，相对于刚体重心的偏移量
> - tag：形状对象的一个标签，你可能还记得，所有的 Node 对象都可以被分配一个 tag，用来进行辨识，实现更容易的访问。形状对象的 tag 作用也一样。

Cocos2d-x 中预定义了这些形状对象：

> - `PhysicsShape`：物理形状的基类
> - `PhysicsShapeCircle`：实心的圆形，无法用它实现一个空心圆
> - `PhysicsShapePolygon`：实心且外凸的多边形
> - `PhysicsShapeBox`：矩形，它是一种特殊的外凸多边形
> - `PhysicsShapeEdgeSegment`：表示一种线段.
> - `PhysicsShapeEdgePolygon`：空心多边形，由多个线段构成的多边形边缘。
> - `PhysicsShapeEdgeBox`：空心矩形，由四个线段组成的矩形边缘
> - `PhysicsShapeEdgeChain`: 链形，它可以有效的把许多边缘连接起来

## 连接/关节[](https://docs.cocos.com/cocos2d-x/manual/zh/physics/concepts.html#%E8%BF%9E%E6%8E%A5%E5%85%B3%E8%8A%82)

**连接(Contacts)** 和 **关节(joint)** 对象描述了刚体相互关联的方式。

## 世界(World)[](https://docs.cocos.com/cocos2d-x/manual/zh/physics/concepts.html#%E4%B8%96%E7%95%8Cworld)

**世界(World)** 是现实物理世界的一个游戏模拟，容纳着所有被添加进去的抽象物体。你可以将刚体，形状，约束都添加到物理世界中，然后将整个世界作为一个整体进行更新。物理世界控制着所有元素的相互作用。其中，用物理 API 实现的许多互动都与 _世界(World)_ 有关。

以上有许多需要记住的东西，请对这些都有个大概印象，以便一会用到的时候随时回看。

## PhysicsWorld[](https://docs.cocos.com/cocos2d-x/manual/zh/physics/concepts.html#physicsworld)

**物理世界(PhysicsWorld)** 是 Cocos2d-x 进行物理模拟的核心对象。物理世界会同时发生很多事情，就像我们生活的世界一样。来想象一个简单的现实场景——厨房，你在思考的时候，就在脑中描绘出了一个厨房的物理世界！厨房世界里拥有一些物体，比如食物，刀具，电器，在这个世界中，这些物体会相互作用。它们会相互接触，并对接触做出反应。比如：用刀子切开食物，并把它放到电器中，做这样一件事。刀子切到食物了吗？可能切到了，也可能还没有，还可能这个刀子根本就不适合做这个。

物理世界(PhysicsWorld)与场景(Scene)进行了深入的整合，只需要调用 `Scene` 对象的 `initWithPhysics()` 方法，就可以创建一个包含物理世界的场景，注意在初始化的时候要进行函数返回值的判断。_`initWithPhysics()` 初始化成功返回 true，失败返回 false_

```
if( !Scene::initWithPhysics() )
{

}
```

每一个 _物理世界(PhysicsWorld)_ 都有与之相关的属性：

> - gravity：全局重力，应用于整个物理世界，默认值为 Vec2(0.0f, -98.0f)
> - speed：物理世界的速度，这里的速度指的是这个模拟世界运行的一种比率，默认值是 1.0
> - updateRate：物理世界的刷新率，这里的刷新率指的是 游戏引擎刷新时间与物理世界刷新时间的比值
> - substeps：物理世界中每次刷新的子步数量

刷新物理世界的过程被称为步进，按照默认设置，物理世界会不停地进行自动刷新，这被称为自动步进。每一帧，都会不停地刷新，你可以通过 `setAutoStep(false)` 禁用一个物理世界的自动步进，然后通过 `PhysicsWorld::step(time)` 设定步进时间来手动刷新物理世界。游戏世界是按帧刷新的，物理世界可以通过子步(substeps)的设置，获得更加频繁的刷新，从而进行更精细的步进控制。

**物理刚体(PhysicsBody)** 对象具有位置和速度，你可以在物理刚体上应用力(forces)，运动(movement)，阻尼(damping)，冲量(impulses)等等。刚体可以是静态的，也可以是动态的，静态的刚体在模拟世界中不会移动，看起来就好像拥有无限大的质量一样，动态的刚体则是一种完全仿真的模拟。刚体可以被玩家手动移动，更常见的是它们受到力的作用而移动。动态刚体可以与所有类型的刚体发生碰撞。Cocos2d-x 提供了 `Node::setPhysicsbody()` 方法实现节点对象和物理刚体对象的关联。

让我们来创建一个静态的物理刚体对象，和五个动态的物理刚体对象，并让五个动态的刚体对象动起来：

```
auto physicsBody = PhysicsBody::createBox(Size(65.0f, 81.0f),
                        PhysicsMaterial(0.1f, 1.0f, 0.0f));
physicsBody->setDynamic(false);

//create a sprite
auto sprite = Sprite::create("whiteSprite.png");
sprite->setPosition(s_centre);
addChild(sprite);

//apply physicsBody to the sprite
sprite->addComponent(physicsBody);

//add five dynamic bodies
for (int i = 0; i < 5; ++i)
{
    physicsBody = PhysicsBody::createBox(Size(65.0f, 81.0f),
                    PhysicsMaterial(0.1f, 1.0f, 0.0f));

    //set the body isn't affected by the physics world's gravitational force
    physicsBody->setGravityEnable(false);

    //set initial velocity of physicsBody
    physicsBody->setVelocity(Vec2(cocos2d::random(-500,500),
                cocos2d::random(-500,500)));
    physicsBody->setTag(DRAG_BODYS_TAG);

    sprite = Sprite::create("blueSprite.png");
    sprite->setPosition(Vec2(s_centre.x + cocos2d::random(-300,300),
                s_centre.y + cocos2d::random(-300,300)));
    sprite->addComponent(physicsBody);

    addChild(sprite);
}
```

结果是，五个动态的物理刚体对象和一个静态的物理刚体对象不断的发生碰撞。如图：

![](https://docs.cocos.com/cocos2d-x/manual/en/physics/physics-img/CorrelationSprite.gif)

# 查询

你肯定有站着一个地方往四周看的经历？你能看到离你近的地方，也能看到离你远的东西，你能判断出它们离你有多远。物理引擎也提供了类似的空间查询功能。

Cocos2d-x 提供的 `PhysicsWorld` 对象支持点查询，射线查询和矩形查询。

## 点查询[](https://docs.cocos.com/cocos2d-x/manual/zh/physics/queries.html#%E7%82%B9%E6%9F%A5%E8%AF%A2)

当你碰到什么东西，比如说你的桌子的时候，你可以将这种情景作为一个点查询的例子。点查询是检查一个点周围的一定距离内是否有物体。通过点查询你可以找到一个物体中距离某定点最近的点，或者找到距离一个定点最近的物体，这非常适合于判断鼠标点击拾取的对象，也可以利用它进行一些其它的简单感知。

## 射线查询[](https://docs.cocos.com/cocos2d-x/manual/zh/physics/queries.html#%E5%B0%84%E7%BA%BF%E6%9F%A5%E8%AF%A2)

当你四处看的时候，在你视线内的某些物体肯定会引起你的注意，你可以将这种情景作为一个射线查询的例子。射线查询是检查从一个定点发出的射线是否相交于一个物体，如果相交可以获取到一个交叉点，这非常适合于判断子弹（忽略子弹的飞行时间）是否命中。

示例：

```
void tick(float dt)
{
    Vec2 d(300 * cosf(_angle), 300 * sinf(_angle));
    Vec2 point2 = s_centre + d;
    if (_drawNode)
    {
        removeChild(_drawNode);
    }
    _drawNode = DrawNode::create();

    Vec2 points[5];
    int num = 0;
    auto func = [&points, &num](PhysicsWorld& world,
        const PhysicsRayCastInfo& info, void* data)->bool
    {
        if (num < 5)
        {
            points[num++] = info.contact;
        }
        return true;
    };

    s_currScene->getPhysicsWorld()->rayCast(func, s_centre, point2, nullptr);

    _drawNode->drawSegment(s_centre, point2, 1, Color4F::RED);
    for (int i = 0; i < num; ++i)
    {
        _drawNode->drawDot(points[i], 3, Color4F(1.0f, 1.0f, 1.0f, 1.0f));
    }
    addChild(_drawNode);

    _angle += 1.5f * (float)M_PI / 180.0f;
}
```

![](https://docs.cocos.com/cocos2d-x/manual/en/physics/physics-img/RayTest.gif)

## 矩形查询[](https://docs.cocos.com/cocos2d-x/manual/zh/physics/queries.html#%E7%9F%A9%E5%BD%A2%E6%9F%A5%E8%AF%A2)

矩形查询提供了一种快速检查区域中有哪些物体的方法，实现起来非常容易：

```
auto func = [](PhysicsWorld& world, PhysicsShape& shape, void* userData)->bool
{
    //Return true from the callback to continue rect queries
    return true;
}

scene->getPhysicsWorld()->queryRect(func, Rect(0,0,200,200), nullptr);
```

这是在制作 Logo 击碎时使用矩形查询的例子：

![](https://docs.cocos.com/cocos2d-x/manual/en/physics/physics-img/rectQuery1.gif)

![](https://docs.cocos.com/cocos2d-x/manual/en/physics/physics-img/rectQuery2.gif)

# 调试

如果你希望在刚体周围绘制红框来帮助调试，那么可以简单的将这两行添加到物理场景的初始化代码中。你当然也可以学习官方测试项目，加一个菜单，在菜单的回调函数里控制是否打开调试功能。

```
Director::getInstance()->getRunningScene()->getPhysics3DWorld()->setDebugDrawEnable(true);
Director::getInstance()->getRunningScene()->setPhysics3DDebugCamera(cameraObjecct);
```

## 禁用物理引擎[](https://docs.cocos.com/cocos2d-x/manual/zh/physics/debugging.html#%E7%A6%81%E7%94%A8%E7%89%A9%E7%90%86%E5%BC%95%E6%93%8E)

使用内置的物理引擎是个好的选择，它稳定又强大。不过，如果你的确想使用一些其它的物理引擎，只需要在 _base/ccConfig.h_ 文件中将 _CC_USE_PHYSICS_ 的值改为 0 禁用内置的物理引擎即可。

# 音乐和音效

你的游戏肯定会需要音乐和音效！Cocos2d-x 提供了一个 **`SimpleAudioEngine`** 类支持游戏内的音乐和音效。它可以被用来增加背景音乐，控制游戏音效。

`SimpleAudioEngine` 是一个共享的单例对象，你可以在代码中的任何地方通过很简单的方式获取到。以下，我们会尽可能的为你展示它的各种使用方法。先来了解一下支持的文件格式。

支持的音乐格式：

|平台|支持的常见文件格式|备注|
|---|---|---|
|Android|mp3, mid, ogg, wav|可以播放android.media.MediaPlayer所支持的所有格式|
|iOS|aac, caf, mp3, m4a, wav|可以播放AVAudioPlayer所支持的所有格式|
|Windows|mid, mp3, wav|无|

支持的音效格式：

|平台|支持的常见文件格式|备注|
|---|---|---|
|Android|ogg, wav|对wav的支持不完美|
|iOS|caf, m4a|可以播放Cocos2d-iPhone CocosDesion所支持的所有格式|
|Windows|mid, wav|无|

# 播放背景音乐

通过下面的方式，播放一个音频文件作为背景音乐，可以控制背景音乐是否循环播放。

```
#include "SimpleAudioEngine.h"
using namespace CocosDenshion;

auto audio = SimpleAudioEngine::getInstance();

// set the background music and continuously play it.
audio->playBackgroundMusic("mymusic.mp3", true);

// set the background music and play it just once.
audio->playBackgroundMusic("mymusic.mp3", false);
```

## 播放音效[](https://docs.cocos.com/cocos2d-x/manual/zh/audio/playing.html#%E6%92%AD%E6%94%BE%E9%9F%B3%E6%95%88)

通过下面的方式，将一个音频文件作为音效。

```
#include "SimpleAudioEngine.h"
using namespace CocosDenshion;

auto audio = SimpleAudioEngine::getInstance();

// play a sound effect, just once.
audio->playEffect("myEffect.mp3", false, 1.0f, 1.0f, 1.0f);
```

# 声音控制

开始播放音乐和音效后，你可能需要对它们进行一些控制，比如暂停、停止、恢复。这很容易完成，下面介绍：

## 暂停[](https://docs.cocos.com/cocos2d-x/manual/zh/audio/operations.html#%E6%9A%82%E5%81%9C)

```
#include "SimpleAudioEngine.h"
using namespace CocosDenshion;

auto audio = SimpleAudioEngine::getInstance();

// pause background music.
audio->pauseBackgroundMusic();

// pause a sound effect.
audio->pauseEffect();

// pause all sound effects.
audio->pauseAllEffects();
```

## 停止[](https://docs.cocos.com/cocos2d-x/manual/zh/audio/operations.html#%E5%81%9C%E6%AD%A2)

```
#include "SimpleAudioEngine.h"
using namespace CocosDenshion;

auto audio = SimpleAudioEngine::getInstance();

// stop background music.
audio->stopBackgroundMusic();

// stop a sound effect.
audio->stopEffect();

// stops all running sound effects.
audio->stopAllEffects();
```

## 恢复[](https://docs.cocos.com/cocos2d-x/manual/zh/audio/operations.html#%E6%81%A2%E5%A4%8D)

```
#include "SimpleAudioEngine.h"
using namespace CocosDenshion;

auto audio = SimpleAudioEngine::getInstance();

// resume background music.
audio->resumeBackgroundMusic();

// resume a sound effect.
audio->resumeEffect();

// resume all sound effects.
audio->resumeAllEffects();
```

# 高级声音功能

## 配置[](https://docs.cocos.com/cocos2d-x/manual/zh/audio/advanced.html#%E9%85%8D%E7%BD%AE)

移动设备上的游戏会遇到一些特殊的情景，比如游戏应用被切换至后台又切换回前台，正在玩游戏的时候电话来了，电话打完继续玩游戏，这些你在进行声音控制的时候都得考虑。

幸运的是，游戏引擎在设计的时候已经考虑到这些情景了，注意在 _AppDelegate.cpp_ 中，有这样几个方法：

```
// This function will be called when the app is inactive. When comes a phone call,
// it's be invoked too
void AppDelegate::applicationDidEnterBackground() {
    Director::getInstance()->stopAnimation();

    // if you use SimpleAudioEngine, it must be pause
    // SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground() {
    Director::getInstance()->startAnimation();

    // if you use SimpleAudioEngine, it must resume here
    // SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
}
```

看到了那些被注释的行吗？如果你有使用 `SimpleAudioEngine` 在游戏中播放声音，记得取消这些注释。当这些被注释的代码生效，你的游戏就能应对刚才提到的场景。

## 预加载[](https://docs.cocos.com/cocos2d-x/manual/zh/audio/advanced.html#%E9%A2%84%E5%8A%A0%E8%BD%BD)

加载音乐和音效通常是个耗时间的过程，为了防止由加载产生的延时导致实际播放与游戏播放不协调的现象，在播放音乐和音效前，可以预加载音乐文件。

```
#include "SimpleAudioEngine.h"
using namespace CocosDenshion;

auto audio = SimpleAudioEngine::getInstance();

// pre-loading background music and effects. You could pre-load
// effects, perhaps on app startup so they are already loaded
// when you want to use them.
audio->preloadBackgroundMusic("myMusic1.mp3");
audio->preloadBackgroundMusic("myMusic2.mp3");

audio->preloadEffect("myEffect1.mp3");
audio->preloadEffect("myEffect2.mp3");

// unload a sound from cache. If you are finished with a sound and
// you wont use it anymore in your game. unload it to free up
// resources.
audio->unloadEffect("myEffect1.mp3");
```

## 音量控制[](https://docs.cocos.com/cocos2d-x/manual/zh/audio/advanced.html#%E9%9F%B3%E9%87%8F%E6%8E%A7%E5%88%B6)

可以像下面这样，通过代码控制音乐和音效的音量：

```
#include "SimpleAudioEngine.h"
using namespace CocosDenshion;

auto audio = SimpleAudioEngine::getInstance();

// setting the volume specifying value as a float
// set default volume
audio->setEffectsVolume(0.5);
audio->setBackgroundMusicVolume(0.5);
```

# 文件系统接入

尽管你可以使用 _stdio.h_ 中的函数来访问文件，但是由于以下原因可能会很不方便：

- 获取文件的绝对路径时，需要调用系统的特定 API
- 安装后，资源文件将打包到 .apk 文件中，绝对路径并不适用
- 想根据屏幕分辨率不同，自动加载不同的分辨率资源，如图片

Cocos2d-x 已经提供了 `FileUtils` 类来解决这些问题。`FileUtils` 是一个用于访问 _Resources_ 目录下文件的帮助类。它也能做一些辅助性的事情，比如检查一个文件是否存在。

## 读文件[](https://docs.cocos.com/cocos2d-x/manual/zh/advanced_topics/filesystem.html#%E8%AF%BB%E6%96%87%E4%BB%B6)

这是一些读文件的函数，不同的函数读不同类型的文件，返回不同的数据类型

|function name|return type|support path type|
|---|---|---|
|getStringFromFile|std::string|relative path and absolute path|
|getDataFromFile|cocos2d::Data|relative path and absolute path|
|getFileDataFromZip|unsigned char*|absolute path|
|getValueMapFromFile|cocos2d::ValueMap|relative path and absolute path|
|getValueVectorFromFile|std::string|cocos2d::ValueVector|

## 管理文件[](https://docs.cocos.com/cocos2d-x/manual/zh/advanced_topics/filesystem.html#%E7%AE%A1%E7%90%86%E6%96%87%E4%BB%B6)

这些函数是用来管理文件，目录的：

|function name|support path type|
|---|---|
|isFileExist|relative path and absolute path|
|isDirectoryExist|relative path and absolute path|
|createDirectory|absolute path|
|removeDirectory|absolute path|
|removeFile|absolute path|
|renameFile|absolute path|
|getFileSize|relative path and absolute path|
# 着色器和材质

## 什么是着色器[](https://docs.cocos.com/cocos2d-x/manual/zh/advanced_topics/shaders.html#%E4%BB%80%E4%B9%88%E6%98%AF%E7%9D%80%E8%89%B2%E5%99%A8)

从维基百科：

在计算机图形学领域，**着色器(Shader)** 是一种特殊类型的计算机程序，最初用于做阴影，在图像中产生适当的光照、明暗，现在主要用于产生特殊效果，也用于视频后期处理。

非专业人士的定义可能是：告诉计算机如何以一种特定的方式绘制东西的程序。简单地说，着色器就是运行在 GPU 上用于图像渲染的一段程序，Cocos2d-x 用它绘制节点。

Cocos2d-x 使用的着色器语言是 [OpenGL ES Shading Language v1.0](https://www.khronos.org/opengles/)，描述 GLSL 语言不在本文的范围之内。想了解更多，请参考 [规范文档](https://www.khronos.org/files/opengles_shading_language.pdf)。在 Cocos2d-x 中，所有的可渲染的 Node 对象都使用着色器。比如，`Sprite` 对象使用为 2D 精灵优化过的着色器，`Sprite3D` 使用为 3D 对象优化过的着色器。

## 自定义着色器[](https://docs.cocos.com/cocos2d-x/manual/zh/advanced_topics/shaders.html#%E8%87%AA%E5%AE%9A%E4%B9%89%E7%9D%80%E8%89%B2%E5%99%A8)

开发者能为任一 Cocos2d-x 的节点对象设置自定义的着色器，添加着色器示例：

```
sprite->setGLProgramState(programState);
sprite3d->setGLProgramState(programState);
```

`GLProgramState` 对象包含两个重要的东西

- `GLProgram`：从根本上来说就是着色器。包含一个顶点着色器和一个像素着色器。
- 状态属性：根本上来说就是着色器的 uniform 变量

如果你不熟悉 uniform 变量也不知道为什么需要它，请参考刚才提到的 [语言规范](https://www.khronos.org/files/opengles_shading_language.pdf)

可以很容易的将 uniform 变量设置到 `GLProgramState`：

```
glProgramState->setUniformFloat("u_progress", 0.9);
glProgramState->setUniformVec2("u_position", Vec2(x,y));
glProgramState->setUniformMat4("u_transform", matrix);
```

你还可以将一个回调函数设置成 uniform 变量，下面是一个 lambda 表达式作为回调函数的例子：

```
glProgramState->setUniformCallback("u_progress", [](GLProgram* glProgram, Uniform* uniform)
{
    float random = CCRANDOM_0_1();
    glProgram->setUniformLocationWith1f(uniform->location, random);
}
);
```

虽然可以手动设置 `GLProgramState` 对象，但更简单的方法是使用材质对象。

## 什么是材质[](https://docs.cocos.com/cocos2d-x/manual/zh/advanced_topics/shaders.html#%E4%BB%80%E4%B9%88%E6%98%AF%E6%9D%90%E8%B4%A8)

设想你想在游戏中画一个这样的球体：

![](https://docs.cocos.com/cocos2d-x/manual/en/advanced_topics/advanced_topics-img/model.jpg)

你要做的第一件事就是定义它的几何形状，像这样：

![](https://docs.cocos.com/cocos2d-x/manual/en/advanced_topics/advanced_topics-img/geometry.jpg)

然后定义砖块纹理，像这样：

![](https://docs.cocos.com/cocos2d-x/manual/en/advanced_topics/advanced_topics-img/brick.jpg)

这样做也能达成目标的效果，但是如果进一步的考虑：

- 如果当球体离相机很远时，想使用质量较低的纹理呢？
- 如果想对砖块应用模糊效果呢？
- 如果想启用或者禁用球体中的照明呢？

答案是使用 **材质(Material)**，而不是使用一个简单的纹理。对于材质，你可以拥有多个纹理，还可以拥有其它的一些特性，比如多重渲染。

材质对象通过 _.material_ 文件创建，其中包含以下信息：

- 材质有一个或多个渲染方法(technique)
- 每个渲染方法有一个或多个通道(pass)
- 每个通道有：
    - 一个渲染状态(RenderState)
    - 一个包含了 uniform 变量的着色器

例如，这是一个材质文件：

```
// A "Material" file can contain one or more materials
material spaceship
{
    // A Material contains one or more Techniques.
    // In case more than one Technique is present, the first one will be the default one
    // A "Technique" describes how the material is going to be renderer
    // Techniques could:
    //  - define the render quality of the model: high quality, low quality, etc.
    //  - lit or unlit an object
    // etc...
    technique normal
    {
        // A technique can contain one or more passes
        // A "Pass" describes the "draws" that will be needed
        //   in order to achieve the desired technique
        // The 3 properties of the Passes are shader, renderState and sampler
        pass 0
        {
            // shader: responsible for the vertex and frag shaders, and its uniforms
            shader
            {
                vertexShader = Shaders3D/3d_position_tex.vert
                fragmentShader = Shaders3D/3d_color_tex.frag

                // uniforms, including samplers go here
                u_color = 0.9,0.8,0.7
                // sampler: the id is the uniform name
                sampler u_sampler0
                {
                    path = Sprite3DTest/boss.png
                    mipmap = true
                    wrapS = CLAMP
                    wrapT = CLAMP
                    minFilter = NEAREST_MIPMAP_LINEAR
                    magFilter = LINEAR
                }
            }
            // renderState: responsible for depth buffer, cullface, stencil, blending, etc.
            renderState
            {
                cullFace = true
                cullFaceSide = FRONT
                depthTest = true
            }
        }
    }
}
```

将一个材质设置到 `Sprite3D` 的方法：

```
Material* material = Material::createWithFilename("Materials/3d_effects.material");
sprite3d->setMaterial(material);
```

如果你想改变不同的渲染方法，你可以这样做：

```
material->setTechnique("normal");
```

### 渲染方法(Technique)[](https://docs.cocos.com/cocos2d-x/manual/zh/advanced_topics/shaders.html#%E6%B8%B2%E6%9F%93%E6%96%B9%E6%B3%95technique)

你只能为一个 `Sprite3D` 绑定一个材质，但这并不意味着固定了一种渲染方式。材质(Material)有一个特性：允许包含多个 **渲染方法(Technique)**，当一个材质被加载时，所有的渲染方法也都被提前加载。有了这个特性，你就可以在运行时方便快速的改变一个对象的渲染效果。

通过使用 `Material::setTechnique(const std::string& name)` 函数，就可以完成渲染方法的切换。这种特性可以用来处理不同灯光的变换，也可以用来处理，在渲染的对象离相机很远时采用质量较低的纹理这种情景。

### 通道(Pass)[](https://docs.cocos.com/cocos2d-x/manual/zh/advanced_topics/shaders.html#%E9%80%9A%E9%81%93pass)

一个渲染方法可以有多个渲染 **通道(Pass)**，其中一个通道对应一次渲染，多通道意味着对一个对象渲染多次，这被称为多通道渲染，也叫多重渲染。每个通道有两个主要的对象：

- `RenderState`：包含 GPU 状态信息，如 _depthTest_, _cullFace_, _stencilTest_，等
- `GLProgramState`：包含要使用的着色器，和一些 uniform 变量

### 材质文件格式[](https://docs.cocos.com/cocos2d-x/manual/zh/advanced_topics/shaders.html#%E6%9D%90%E8%B4%A8%E6%96%87%E4%BB%B6%E6%A0%BC%E5%BC%8F)

Cocos2d-x 的材质文件使用一种优化过的文件格式，同时与其它一些开源引擎的材质文件格式类似，如 _GamePlay3D_，_OGRE3D_。

注意点：

- 材质文件的扩展名无关紧要，建议使用 _.material_ 作为扩展名
- 顶点着色器和像素着色器的文件扩展名也无关紧要，建议使用 _.vert_ 和 _.frag_
- _id_ 是材质(Meterial)，渲染方法(technique)，通道(pass)的可选属性
- 材质可以通过设置 _parent_material_id_ 继承其它材质的值

```
// When the .material file contains one material
sprite3D->setMaterial("Materials/box.material");
// When the .material file contains multiple materials
sprite3D->setMaterial("Materials/circle.material#wood");
```

#### 字段定义[](https://docs.cocos.com/cocos2d-x/manual/zh/advanced_topics/shaders.html#%E5%AD%97%E6%AE%B5%E5%AE%9A%E4%B9%89)

|   |   |   |
|---|---|---|
|material material_id : parent_material_id|||
|{|||
|renderState {}|[0..1]|block|
|technique id {}|[0..*]|block|
|}|||

|   |   |   |
|---|---|---|
|technique technique_id|||
|{|||
|renderState {}|[0..1]|block|
|pass id {}|[0..*]|block|
|}|||

|   |   |   |
|---|---|---|
|pass pass_id|||
|{|||
|renderState {}|[0..1]|block|
|shader {}|[0..1]|block|
|}|||

|   |   |   |
|---|---|---|
|renderState|||
|{|||
|blend = false|[0..1]|bool|
|blendSrc = [BLEND_ENUM](https://docs.cocos.com/cocos2d-x/manual/zh/advanced_topics/shaders.html#BLEND_ENUM)|[0..1]|enum|
|blendDst = [BLEND_ENUM](https://docs.cocos.com/cocos2d-x/manual/zh/advanced_topics/shaders.html#BLEND_ENUM)|[0..1]|enum|
|cullFace = false|[0..1]|bool|
|depthTest = false|[0..1]|bool|
|depthWrite = false|[0..1]|bool|
|}|||
|frontFace = CW \| CCW|[0..1]|enum|
|depthTest = false|[0..1]|bool|
|depthWrite = false|[0..1]|bool|
|depthFunc = [FUNC_ENUM](https://docs.cocos.com/cocos2d-x/manual/zh/advanced_topics/shaders.html#wiki-FUNC_ENUM)|[0..1]|enum|
|stencilTest = false|[0..1]|bool|
|stencilWrite = 4294967295|[0..1]|uint|
|stencilFunc = [FUNC_ENUM](https://docs.cocos.com/cocos2d-x/manual/zh/advanced_topics/shaders.html#wiki-FUNC_ENUM)|[0..1]|enum|
|stencilFuncRef = 0|[0..1]|int|
|stencilFuncMask = 4294967295|[0..1]|uint|
|stencilOpSfail = [STENCIL_OPERATION_ENUM](https://docs.cocos.com/cocos2d-x/manual/zh/advanced_topics/shaders.html#wiki-STENCIL_OP_ENUM)|[0..1]|enum|
|stencilOpDpfail = [STENCIL_OPERATION_ENUM](https://docs.cocos.com/cocos2d-x/manual/zh/advanced_topics/shaders.html#wiki-STENCIL_OP_ENUM)|[0..1]|enum|
|stencilOpDppass = [STENCIL_OPERATION_ENUM](https://docs.cocos.com/cocos2d-x/manual/zh/advanced_topics/shaders.html#wiki-STENCIL_OP_ENUM)|[0..1]|enum|

|   |   |   |
|---|---|---|
|shadershader_id|||
|{|||
|vertexShader = res/colored.vert|[0..1]|file path|
|fragmentShader = res/colored.frag|[0..1]|file path|
|defines = semicolon separated list|[0..1]|string|
||||
|uniform_name = [scalar](https://docs.cocos.com/cocos2d-x/manual/zh/advanced_topics/shaders.html#scalar) \| [vector](https://docs.cocos.com/cocos2d-x/manual/zh/advanced_topics/shaders.html#vector)|[0..*]|uniform|
|uniform_name = [AUTO_BIND_ENUM](https://docs.cocos.com/cocos2d-x/manual/zh/advanced_topics/shaders.html#AUTO_BIND_ENUM)|[0..*]|enum|
|sampler uniform_name {}|[0..*]|block|
|}|||

|   |   |   |
|---|---|---|
|sampler uniform_name|||
|{|||
|path = res/wood.png \| @wood|[0..1]|image path|
|mipmap = bool|[0..1]|bool|
|wrapS = REPEAT \| CLAMP|[0..1]|enum|
|wrapT = REPEAT \| CLAMP|[0..1]|enum|
|minFilter = [TEXTURE_MIN_FILTER_ENUM](https://docs.cocos.com/cocos2d-x/manual/zh/advanced_topics/shaders.html#TEXTURE_MIN_FILTER_ENUM)|[0..1]|enum|
|magFilter = [TEXTURE_MAG_FILTER_ENUM](https://docs.cocos.com/cocos2d-x/manual/zh/advanced_topics/shaders.html#TEXTURE_MAG_FILTER_ENUM)|[0..1]|enum|
|}|||

#### 枚举类型定义[](https://docs.cocos.com/cocos2d-x/manual/zh/advanced_topics/shaders.html#%E6%9E%9A%E4%B8%BE%E7%B1%BB%E5%9E%8B%E5%AE%9A%E4%B9%89)

|**TEXTURE_MIN_FILTER_ENUM**||
|:--|---|
|NEAREST|Lowest quality non-mipmapped|
|LINEAR|Better quality non-mipmapped|
|NEAREST_MIPMAP_NEAREST|Fast but low quality mipmapping|
|LINEAR_MIPMAP_NEAREST||
|NEAREST_MIPMAP_LINEAR||
|LINEAR_MIPMAP_LINEAR|Best quality mipmapping|

|**TEXTURE_MAG_FILTER_ENUM**||
|:--|---|
|NEAREST|Lowest quality|
|LINEAR|Better quality|

|**BLEND_ENUM**||
|:--|---|
|ZERO|ONE_MINUS_DST_ALPHA|
|ONE|CONSTANT_ALPHA|
|SRC_ALPHA|ONE_MINUS_CONSTANT_ALPHA|
|ONE_MINUS_SRC_ALPHA|SRC_ALPHA_SATURATE|
|DST_ALPHA||

|**CULL_FACE_SIDE_ENUM**||
|:--|---|
|BACK|Cull back-facing polygons.|
|FRONT|Cull front-facing polygons.|
|FRONT_AND_BACK|Cull front and back-facing polygons.|

|**FUNC_ENUM**||
|:--|---|
|NEVER|ALWAYS|
|LESS|GREATER|
|EQUAL|NOTEQUAL|
|LEQUAL|GEQUAL|

|**STENCIL_OPERATION_ENUM**||
|:--|---|
|KEEP|REPLACE|
|ZERO|INVERT|
|INCR|DECR|
|INCR_WRAP|DECR_WRAP|

**数据类型**:

- scalar 代表标量，可以用浮点型(float)，整形(int)，布尔型(bool)
- vector 代表矢量，用逗号分隔的一系列浮点数表示

### 预定义的 uniform 变量[](https://docs.cocos.com/cocos2d-x/manual/zh/advanced_topics/shaders.html#%E9%A2%84%E5%AE%9A%E4%B9%89%E7%9A%84-uniform-%E5%8F%98%E9%87%8F)

下面是 Cocos2d-x 预定义的一些 uniform 变量，你可以在自定义的着色器中使用它们。

- `CC_PMatrix`: A `mat4` with the projection matrix
- `CC_MVMatrix`: A `mat4` with the Model View matrix
- `CC_MVPMatrix`: A `mat4` with the Model View Projection matrix
- `CC_NormalMatrix`: A `mat4` with Normal Matrix
- `CC_Time`: a `vec4` with the elapsed time since the game was started
    - CC_Time[0] = time / 10;
    - CC_Time[1] = time;
    - CC_Time[2] = time * 2;
    - CC_Time[3] = time * 4;
- `CC_SinTime`: a `vec4` with the elapsed time since the game was started:
    - CC_SinTime[0] = time / 8;
    - CC_SinTime[1] = time / 4;
    - CC_SinTime[2] = time / 2;
    - CC_SinTime[3] = sinf(time);
- `CC_CosTime`: a `vec4` with the elapsed time since the game was started:
    - CC_CosTime[0] = time / 8;
    - CC_CosTime[1] = time / 4;
    - CC_CosTime[2] = time / 2;
    - CC_CosTime[3] = cosf(time);
- `CC_Random01`: A `vec4` with four random numbers between 0.0f and 1.0f
- `CC_Texture0`: A `sampler2D`
- `CC_Texture1`: A `sampler2D`
- `CC_Texture2`: A `sampler2D`
- `CC_Texture3`: A `sampler2D`

# 图形性能优化

## 黄金法则[](https://docs.cocos.com/cocos2d-x/manual/zh/advanced_topics/optimizing.html#%E9%BB%84%E9%87%91%E6%B3%95%E5%88%99)

### 二八原则[](https://docs.cocos.com/cocos2d-x/manual/zh/advanced_topics/optimizing.html#%E4%BA%8C%E5%85%AB%E5%8E%9F%E5%88%99)

系统中 20% 的代码会消耗 80% 的性能！在进行性能优化时，我们应该始终坚持这个原则。

### 够用原则[](https://docs.cocos.com/cocos2d-x/manual/zh/advanced_topics/optimizing.html#%E5%A4%9F%E7%94%A8%E5%8E%9F%E5%88%99)

如果有两种方式渲染图像，无法观察出哪个渲染的效果更好，那就选用性能消耗更低的方式。我们知道，RGBA4444 像素格式的 _PNG_ 图像质量比 RGBA8888 像素格式的要低，但是如果在游戏效果上，无法观察出哪个效果好，我们应该坚持使用 RGBA4444 的像素格式，因为它占用更少的内存，出现内存问题和带宽问题的可能性更小。

音频采样率也是一样的。

### 了解目标设备和游戏引擎[](https://docs.cocos.com/cocos2d-x/manual/zh/advanced_topics/optimizing.html#%E4%BA%86%E8%A7%A3%E7%9B%AE%E6%A0%87%E8%AE%BE%E5%A4%87%E5%92%8C%E6%B8%B8%E6%88%8F%E5%BC%95%E6%93%8E)

了解目标设备的 CPU/GPU 系列，当性能问题仅在某些设备上出现时，这个信息就非常重要。或许你会发现他们共用一种 GPU：ARM、PowerVR 或 Mali。然后就可以进行有针对性的分析。

了解目前使用的游戏引擎也很重要，如果你知道引擎是如何组织图形命令，如何处理绘制过程。那在编码的过程中就能避免许多常见的陷阱。

### 使用工具分析[](https://docs.cocos.com/cocos2d-x/manual/zh/advanced_topics/optimizing.html#%E4%BD%BF%E7%94%A8%E5%B7%A5%E5%85%B7%E5%88%86%E6%9E%90)

有许多工具可用于分析图形性能，即使我们需要优化 Android 游戏的性能，也可以使用 Xcode 帮助调试。

- Xcode: [Debugging-OpenGL-ES-With-Xcode-Profile-Tools](https://github.com/rstrahl/rudistrahl.me/blob/master/entries/Debugging-OpenGL-ES-With-Xcode-Profile-Tools.md)
- 官方文档: [OpenGLES_ProgrammingGuide](https://developer.apple.com/library/ios/documentation/3DDrawing/Conceptual/OpenGLES_ProgrammingGuide/ToolsOverview/ToolsOverview.html)

三大移动 GPU 供应商，也提供了图形分析工具

- ARM Mali GPU: [mali-graphics-debugge](http://malideveloper.arm.com/resources/tools/mali-graphics-debugger/)
- Imagination PowerVR GPU: [pvrtune](https://community.imgtec.com/developers/powervr/tools/pvrtune/)
- Qualcomm Adreno GPU: [adreno-gpu-profiler](https://developer.qualcomm.com/software/adreno-gpu-profiler)

当你遇到图形性能问题时可以使用这些工具，**但在这之前，请先确认是不是 CPU 导致的性能问题**，注意不要随意猜测，而是要通过分析得出结果。

## 常见瓶颈[](https://docs.cocos.com/cocos2d-x/manual/zh/advanced_topics/optimizing.html#%E5%B8%B8%E8%A7%81%E7%93%B6%E9%A2%88)

作为一个经验法则，游戏性能问题更容易出现在 CPU ，而不是 GPU

### CPU 性能优化[](https://docs.cocos.com/cocos2d-x/manual/zh/advanced_topics/optimizing.html#cpu-%E6%80%A7%E8%83%BD%E4%BC%98%E5%8C%96)

**绘制调用(draw call)** 次数过多，游戏循环中计算量过大，都会造成 CPU 性能下降，尽量减少游戏中的总绘制调用次数，我们应该尽可能的使用批量绘制。 Cocos2d-x 有自动批量处理绘制的支持，但仍需要一些努力才能使其工作。

当玩家玩游戏时，要尽量避免 IO 操作，尽可能预加载图集、音频、TTF字体等

更不要在游戏循环中进行繁重的计算操作，因为这可能造成每帧 60 次的大量计算，性能消耗非常恐怖！

### GPU 性能优化[](https://docs.cocos.com/cocos2d-x/manual/zh/advanced_topics/optimizing.html#gpu-%E6%80%A7%E8%83%BD%E4%BC%98%E5%8C%96)

如果只是在开发一个 2D 游戏，也没有写复杂的着色器，那基本不会遇到 GPU 性能问题。但是过度绘制的问题仍然存在，如果过度绘制较多，将会消耗大量带宽，进而降低 GPU 性能。

尽管现在移动 GPU 具有 _TBDR(基于平铺的渲染)_ 架构，但是只有 PowerVR 的 _HSR(隐藏曲面去除)_ 可以显著减少过度绘制问题，其它 GPU 仅执行 TBDR 和早期的 z 测试，只有在提交不透明的几何图形时才能减少过渡绘制问题。

Cocos2d-x 总是按照从后向前的规则提交绘制命令，这样在 2D 中即使有许多透明图像，也能保证正确的混合效果。

## Cocos2d-x 性能优化建议[](https://docs.cocos.com/cocos2d-x/manual/zh/advanced_topics/optimizing.html#cocos2d-x-%E6%80%A7%E8%83%BD%E4%BC%98%E5%8C%96%E5%BB%BA%E8%AE%AE)

1. 始终使用批量绘图，将同一图层中的精灵图像打包成一个大的图集
2. 根据经验，尽量保持 _绘制调用(draw call)_ 次数低于 50，总之尽量减少就对了！
3. 在 原始 32 位（RGBA8888）纹理上，优先使用 16 位（RGBA4444 + 抖动）的处理方式
4. 使用压缩纹理，在 iOS 中使用 PVRTC 纹理，在 Android 平台上，使用 ETC1，但是 ETC1 没有 alpha 通道，你可能需要编写自定义着色器并为 alpha 通道提供单独的 ETC1 图像
5. 不要使用系统字体作为您的游戏得分计数器，它很慢的，尝试使用 TTF 或 BMFont，BMFont 更快
6. 尝试在使用音频和其它游戏对象前，进行预加载
7. 使用 armabi-v7a 构建 Android 工程，这会有更好的性能表现
8. 使用烘焙光照，而不是动态光照
9. 避免使用复杂的像素着色器
10. 避免在像素着色器中使用丢弃和 alpha 测试，它会影响 HSR 优化

