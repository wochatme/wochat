# ZTerm的文档加密设计

对于知识库中的文档，我们加密后统一存放在https://zterm.ai/t 目录下，共用户下载。

每一个文档有一个文档id，该id就是sha256对加密后的文档进行哈希后得到的32字节的值，变成字符串就是64字节。 每个文档的加密密钥K是一个32字节的随机数。K保存在数据库中的doc表中。doc表的结构如下：
```
CREATE TABLE doc
(
	did CHAR(64);
	key CHAR(64);
)
```

## 私钥的产生
系统的安全由一个单一的私钥SK来保证。SK是随机产生的一个32字节的随机数。 我们使用工具genkey来产生它。
```
$ ./genkey
Usage: ./genkey password

$ ./genkey ZTerm@AI
You password is:[ZTerm@AI]
Secret Key : 597fc88bc45aa52b8a177d10bde683f8e4d0acb4475d73c1806d3ab2e261a2a2
Public Key : 02563e36e55c39dcf6fda6ae5ecf75e2384d8455e61ac973f8cddac44133a901b2  <--------------------- 公钥
```

工具genkey产生一个32字节的随机数SK作为私钥。但是SK是由口令保护的。 上述命令行输入的"ZTerm@AI"即为保护私钥SK的口令。 最终产生的私钥SK的值为："597fc88bc45aa52b8a177d10bde683f8e4d0acb4475d73c1806d3ab2e261a2a2"。 当然这是加密后的私钥。它和口令两个信息就可以解密出真正的私钥。这是整个安全的核心要点。


## 私钥的验证
我们可以使用verifykey工具来验证上述加密后的私钥SK，使用方式如下：
```
$ ./verifykey
Usage: ./verifykey password secret_key_string

$ ./verifykey ZTerm@AI 597fc88bc45aa52b8a177d10bde683f8e4d0acb4475d73c1806d3ab2e261a2a2
Public Key : 02563e36e55c39dcf6fda6ae5ecf75e2384d8455e61ac973f8cddac44133a901b2   <----------------------- 公钥
```

验证工具verifykey有两个参数：口令和加密后私钥字符串。 该工具解密私钥后，产生并显示该私钥对应的公钥。 你可以看到verifykey显示的公钥和genkey显示的公钥是完全一样的，说明读取正确。


## 加密文件
假设我们有一个文件要加密，可以使用encfile工具来对它进行加密。enfile的输入参数有4个：
- 私钥的口令
- 加密后的私钥字符串
- 私钥对应的公钥字符串，用于验证私钥的正确性
- 被加密的文件

其中最后一个参数是可选项。如果不指定要加密的文件，encfile仅仅是验证私钥和公钥是否匹配。 使用方法如下：
```
$ ./encfile
Usage: ./encfile password secretkey pubkey [file_name]

$ ./encfile ZTerm@AI 597fc88bc45aa52b8a177d10bde683f8e4d0acb4475d73c1806d3ab2e261a2a2 02563e36e55c39dcf6fda6ae5ecf75e2384d8455e61ac973f8cddac44133a901b2
Secret key and public key are matched!
Secret Key : 597fc88bc45aa52b8a177d10bde683f8e4d0acb4475d73c1806d3ab2e261a2a2
Public Key : 02563e36e55c39dcf6fda6ae5ecf75e2384d8455e61ac973f8cddac44133a901b2
```

在上面的用例中，我们没有指定要加密哪个文件，所以它只是验证一下解密后的私钥和提供的公钥是否匹配，结果两者是匹配的。

下面我们加密一个文件，使用方法如下：
```
$ ./encfile ZTerm@AI 597fc88bc45aa52b8a177d10bde683f8e4d0acb4475d73c1806d3ab2e261a2a2 02563e36e55c39dcf6fda6ae5ecf75e2384d8455e61ac973f8cddac44133a901b2 HowToBuildZTerm.md
Generate encrypted file: [f0bba895ed291f4f30cdbad08b77dd2faee88d03b356306e0c00670d10681920]
INSERT INTO doc(did,key) VALUES(
        'f0bba895ed291f4f30cdbad08b77dd2faee88d03b356306e0c00670d10681920',
        '4f9ece6574631a48cca8c0cff2b4c6afd242baab678d2d5c626a6982e2da94c7'
);
```

经过上述操作，文档HowToBuildZTerm被加密成了新文件，它的文件名是：f0bba895ed291f4f30cdbad08b77dd2faee88d03b356306e0c00670d10681920。 把这个文件放在https://zterm.ai/t 目录下供用户下载即可。

同时该工具还生成了一条INSERT的SQL语句，在数据库中执行这条语句把密钥存入到数据库中即可。 当然4f9ece6574631a48cca8c0cff2b4c6afd242baab678d2d5c626a6982e2da94c7是被私钥加密过的。 即使数据库被黑客攻破，拿到了doc表里面的信息，只要私钥SK没有泄漏，黑客也无法破译每个文档的密钥。 所以保护私钥是整个系统唯一需要注意的地方。

如果输入5个参数，则会显示真正加密文档的密钥。 我们多输入了一个参数x，这个无所谓，只要凑够五个参数就可以了。
```
$ ./encfile ZTerm@AI 597fc88bc45aa52b8a177d10bde683f8e4d0acb4475d73c1806d3ab2e261a2a2 02563e36e55c39dcf6fda6ae5ecf75e2384d8455e61ac973f8cddac44133a901b2 HowToBuildZTerm.md x
REAL KEY : 57a927a3ed425685036e11180112390b4f97869da386eca4b82721bb657e92bc                  <---------------- 真正加密文档的密钥
Generate encrypted file: [0c10db2d93d5d5544e9e62764702cd309d6537856b4af305205649baba86b1a1]
INSERT INTO doc(did,key) VALUES(
        '0c10db2d93d5d5544e9e62764702cd309d6537856b4af305205649baba86b1a1',
        '242f2712a334d9e665ee2bbafdb24f5c08c2a8630e3d752eb21295edacd818e9'
);
```

上面的输出中，REAL KEY表示未经加密的密钥。


## 产生用户需要的密钥

当用户索要某一个文档的解密密钥时，使用filekey工具，它的使用方法如下：
```
$ ./filekey
Usage: ./filekey password secretkey filekey [pubkey]
```
上述四个参数的含义如下：
- 第一个参数是私钥的口令
- 第二个参数是加密后的私钥
- 第三个是保存在数据库doc表中key列的值。 
- 第四个是用户的公钥，用户索取文档密钥时，必须提供自己的公钥。服务器拿着他的公钥对文档密钥进行加密，只有该用户才能解密。

如果不指定用户的公钥，则执行的是解密文档密钥的工作。我们执行：

```
$ ./filekey ZTerm@AI 597fc88bc45aa52b8a177d10bde683f8e4d0acb4475d73c1806d3ab2e261a2a2 242f2712a334d9e665ee2bbafdb24f5c08c2a8630e3d752eb21295edacd818e9
REAL KEY : 57a927a3ed425685036e11180112390b4f97869da386eca4b82721bb657e92bc
```
你注意对比上面的REAL KEY，两者是一模一样的，说明filekey可以正确解密文档的密钥。

如果某一个用户索要该密钥，需要提供自己的公钥。 服务器会拿自己的私钥和用户的公钥计算出一个Kp，32字节，用这个Kp来加密真正的文档密钥。用户端可以利用自己的私钥和服务器的公钥同样计算出Kp，从而实现了解密。

```
$ ./filekey ZTerm@AI 597fc88bc45aa52b8a177d10bde683f8e4d0acb4475d73c1806d3ab2e261a2a2 242f2712a334d9e665ee2bbafdb24f5c08c2a8630e3d752eb21295edacd818e9 039bcc65e9dd5878d076de35daf8c9c03e42b99fe9a56d5e14caf47afeb4b92f8a
Key To User : 3be61ba0c8a5127e0297e844271761e5c6a38742c5f24d14060f394bd7a6340d
```

最终产生的该文档的密钥是“3be61ba0c8a5127e0297e844271761e5c6a38742c5f24d14060f394bd7a6340d”，这个密钥必须是拥有公钥"039bcc65e9dd5878d076de35daf8c9c03e42b99fe9a56d5e14caf47afeb4b92f8a"对应私钥的用户才能够解开。 这种机制保证了这把密钥不会被别人使用。

## 反馈：
有任何问题，发信到support@zterm.ai




