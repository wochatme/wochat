# ZTerm的加密设计

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
Secret Key : 30ef272a10d1302134801d6f8d27ad72ffe42c5544710769ec491ac7f5c9daea
Public Key : 026c33b66d1ba4f39b350a12d159f107305bc6aafd1b7369bd77f304fafe21e5a2  <--------------------- 公钥
```
工具genkey产生一个32字节的随机数SK作为私钥。但是SK是由口令保护的。 上述命令行输入的"ZTerm@AI"即为保护私钥SK的口令。 最终产生的私钥SK的值为："30ef272a10d1302134801d6f8d27ad72ffe42c5544710769ec491ac7f5c9daea"。 当然这是加密后的私钥。它和口令两个信息就可以解密出真正的私钥。


## 私钥的验证
我们可以使用verifykey工具来验证上述加密后的私钥SK，使用方式如下：
```
$ ./verifykey
Usage: ./verifykey password secret_key_string
$ ./verifykey ZTerm@AI 30ef272a10d1302134801d6f8d27ad72ffe42c5544710769ec491ac7f5c9daea
Public Key : 026c33b66d1ba4f39b350a12d159f107305bc6aafd1b7369bd77f304fafe21e5a2   <----------------------- 公钥
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

$ ./encfile ZTerm@AI 30ef272a10d1302134801d6f8d27ad72ffe42c5544710769ec491ac7f5c9daea 026c33b66d1ba4f39b350a12d159f107305bc6aafd1b7369bd77f304fafe21e5a2
Secret key and public key are matched!
```

在上面的用例中，我们没有指定要加密哪个文件，所以它只是验证一下解密后的私钥和提供的公钥是否匹配，结果两者是匹配的。

下面我们加密一个文件，使用方法如下：
```
$ ./encfile ZTerm@AI 30ef272a10d1302134801d6f8d27ad72ffe42c5544710769ec491ac7f5c9daea 026c33b66d1ba4f39b350a12d159f107305bc6aafd1b7369bd77f304fafe21e5a2 HowToBuildZTerm.md
Generate encrypted file: [9f9facac29e1b374acb1d6dfd0bb0937e0147ddfc56551941e83c31beda535e5]
INSERT INTO doc(did,key) VALUES('9f9facac29e1b374acb1d6dfd0bb0937e0147ddfc56551941e83c31beda535e5', 'c762ded647a7836d202f40df64f7fc72666251172bdddce40fde5fb67c283a8a');
```

经过上述操作，文档HowToBuildZTerm.9f9facac29e1b374acb1d6dfd0bb0937e0147ddfc56551941e83c31beda535e5。 把这个文件放在https://zterm.ai/t 目录下供用户下载即可。

同时该工具还生成了一条INSERT的SQL语句，在数据库中执行这条语句把密钥出入到数据库中即可。 当然c762ded647a7836d202f40df64f7fc72666251172bdddce40fde5fb67c283a8a是被私钥加密过的。 即使数据库被黑客攻破，拿到了doc表里面的信息，只要私钥SK没有泄漏，黑客也无法破译每个文档的密钥。 所以保护私钥是整个系统唯一需要注意的地方。

## 产生用户需要的密钥

当用户索要某一个文档的解密密钥时，使用filekey工具，它的使用方法如下：
```
$ ./filekey
Usage: ./filekey password secretkey pubkey filekey
```
第一个参数是私钥的口令，第二个参数是加密后的私钥，第三个是用户的公钥，第四个是保存在数据库doc表中key列的值。 我们执行：
```
$ ./filekey ZTerm@AI 30ef272a10d1302134801d6f8d27ad72ffe42c5544710769ec491ac7f5c9daea 02b915c65202d35eeaf8760827499042bef0f2fc88009bc0d17d9949362b36686f c762ded647a7836d202f40df64f7fc72666251172bdddce40fde5fb67c283a8a
Key To User : 4508a7da2a0823bdf1c3174888a75888d34f2e8beed44a7298f2f43e0bc64905
```
最终产生的该文档的密钥，必须是拥有公钥"02b915c65202d35eeaf8760827499042bef0f2fc88009bc0d17d9949362b36686f"对应私钥的用户才能够解开“4508a7da2a0823bdf1c3174888a75888d34f2e8beed44a7298f2f43e0bc64905”，获得该文档的解密密钥。




