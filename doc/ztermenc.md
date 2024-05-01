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
Usage: ./genkey password keyfile
$ ./genkey ZTerm@AI key.bin
Generate key file successfully:[key.bin]
Pubkey : 0393e853964539ec9d22eca7d5bd270645608dee136951681330be81b7e9b0439c
$ ls -l key.bin
-rwxrwxrwx 1 kevin kevin 32 May  1 11:05 key.bin
```
工具genkey产生一个32字节的二进制文件key.bin，在它里面保存了私钥SK。但是SK是由口令保护的。 上述命令行输入的"ZTerm@AI"即为保护私钥SK的口令。 每次访问key.bin，必须提供口令，才能够正确地解密SK。

## 私钥的验证
我们可以使用verify工具来读取key.bin，获得该SK对应的公钥，使用如下：
```
$ ./verify
Usage: ./verify password sk_file
$ ./verify ZTerm@AI key.bin
Pubkey : 0393e853964539ec9d22eca7d5bd270645608dee136951681330be81b7e9b0439c
```
同样，verify有两个参数：口令和私钥文件。读取后，显示该私钥对应的公钥。 你可以看到verify显示的公钥和genkey显示的公钥是完全一样的，说明读取正确。

## 加密文件
假设我们有一个文件要加密，可以使用encfile工具来对它进行加密。enfile的输入参数有4个：
- 私钥文件的口令
- 私钥文件
- 私钥文件对应的公钥，用于验证私钥的正确性
- 被加密的文件

使用方法如下：
```
$ ./encfile
Usage: ./encfile password sk_file pubkey file_name
$ ./encfile ZTerm@AI key.bin 0393e853964539ec9d22eca7d5bd270645608dee136951681330be81b7e9b0439c HowToBuildZTerm.md
SK and PK are both good
Do compress![10431]
deflate = Z_STREAM_END [6996 : 10431, 67.000000]
Random data:
ca002b4a28ccba038f4cc621d09efe5d0bbfb45838b6cb45dc3307daa5d1b23b
Key data:
9824499bb41e87e347251ab88da54dede5effd53fb00d600c85e62e63d476536     <------------------------------------ 真正加密文件的key
Generate data file: cb87958d918fcb9aa81e72d2690f3ca8605aa5d6dc618271494e3802d0d3de30
Generate key file:  cb87958d918fcb9aa81e72d2690f3ca8605aa5d6dc618271494e3802d0d3de30.txt
Key : 5c68f11b1482945171dc1ab5fc3f823a04dcc9a2b899dce38a448322779c5541 <---------------------------------- 被私钥加密后的key
Convert is successfully!
$ cat cb87958d918fcb9aa81e72d2690f3ca8605aa5d6dc618271494e3802d0d3de30.txt
5c68f11b1482945171dc1ab5fc3f823a04dcc9a2b899dce38a448322779c5541 <---------------------------------- 被私钥加密后的key
$
```
经过上述操作，文档HowToBuildZTerm.md被压缩，加密，产生的文件是cb87958d918fcb9aa81e72d2690f3ca8605aa5d6dc618271494e3802d0d3de30。 把这个文件放在https://zterm.ai/t 目录下供用户下载即可。和加密后的文件伴随的是对应的密钥文件cb87958d918fcb9aa81e72d2690f3ca8605aa5d6dc618271494e3802d0d3de30.txt。 它里面包含了被私钥加密后的文件密钥。

被私钥加密后的密钥是5c68f11b1482945171dc1ab5fc3f823a04dcc9a2b899dce38a448322779c5541，我们把它存放在表doc中，执行如下SQL:
```
INSERT doc(did, key) VALUES('cb87958d918fcb9aa81e72d2690f3ca8605aa5d6dc618271494e3802d0d3de30','5c68f11b1482945171dc1ab5fc3f823a04dcc9a2b899dce38a448322779c5541');
```
即使数据库被黑客攻破，拿到了doc里面的信息，只要私钥SK没有泄漏，就不会有严重的后果。

## 解密文件的密钥。

当用户索取某一个文档的密钥时，我们必须使用私钥对doc中的key的信息进行解密。工具是extract，使用方法如下：
```
$ ./extract
Usage: ./extract password sk_file keystring
$ ./extract ZTerm@AI key.bin 5c68f11b1482945171dc1ab5fc3f823a04dcc9a2b899dce38a448322779c5541
Pubkey : 0393e853964539ec9d22eca7d5bd270645608dee136951681330be81b7e9b0439c
REAL KEY : 9824499bb41e87e347251ab88da54dede5effd53fb00d600c85e62e63d476536  <----------  真正加密和解密文档的密钥
```

我们拿到加密和解密文档的真正密钥Kr(REAL KEY)以后，需要使用服务器的私钥和用户的公钥产生的密钥Kp对Kr进行加密。这样，只有拥有该用户公钥对应的私钥的人才能解密文档。 其关系如下：
```
Kp = f(SK, PK)
KK = g(Kp, Kr)
```
其中SK是服务器的私钥，PK是用户索要文档密钥是提供的他自己的公钥。

工具filekey完成这个过程：
```
$ ./filekey
Usage: ./file password sk_file real_key
```



