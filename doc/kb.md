# 知识库的设计

目前知识库是PostgreSQL数据库中的一张表，名字叫做doc，表示document的意思。它的具体定义如下：

```
DROP TABLE doc;
CREATE TABLE doc (
	idx BIGSERIAL PRIMARY KEY,
	dte TIMESTAMP WITHOUT TIME ZONE DEFAULT now(),
	ftp INTEGER DEFAULT 1,
	ser BIGINT DEFAULT 0,
	num SMALLINT DEFAULT 1,
	did CHAR(16),
	key CHAR(64),
	tle VARCHAR(256),
	txt TEXT,
	kwd TEXT,
	ref TEXT
);

GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA public TO ubuntu;
GRANT USAGE, SELECT ON SEQUENCE doc_idx_seq TO ubuntu;
CREATE INDEX doc_hash ON doc USING HASH(did);
```

这张表中最重要的列是txt列，它里面存储了原始文档。原始文档的内容是自包含的，即：本文档内容的完整性不依赖任何外部信息。 文档是纯文本的，尽量采用MarkDown格式。开始两行的内容是统一的，模板如下：
```
# This Is The Title of This Document
https://www.postgresql.org/docs/current/datatype-character.html
```

我们可以看出，第一行是一个#，后面跟着这篇文档的标题。文档入库程序在把一篇新文档插入到doc表中时，会解析这两行，并把文档标题更新到tle这一列。tle这一列包含的是文档标题。 第二行是这个文档的原始出处，可能没有，就是我们原创的文档。 如果文档中包含图片，图片是以base64格式的形式嵌入到文档中，保证文档的内容的自包含性，即：不再依赖任何外部的信息。如果文档中包含SVG格式，也同样是嵌入到文档中，保证文档的自包含性。

本表的各列具体含义解释如下：

- idx : 每次增加一的序列号，唯一性地表示一篇文档。譬如它的值是：1,2,3,4,5,6...不断增加。这一列是主键(primary key)。
- dte : 文档入库或者修改后的时间戳。
- ftp : file type ，文档的类型，缺省值为1，表示文本文件。 值2表示我们私有的视频格式。
- ser : 文档可能是一个系列，在这种情况下，ser指向了idx列中的某一个值，该值锁定的文档即为该系列文档的第一篇。缺省值为0，表示该文档不属于一个系列，独立成篇。
- num : 如果该文档属于某一个系列，则本列表示本文档是该系列文档的第几篇。一个系列的文档的第一篇的编号是1，最多有256篇。 本列的缺省值为1。
- did : 文档加密后的标识，具有唯一性，为16个字节。
- key : 文档的加密密钥。
- tle : 文档的标题。 这个信息是在文档入口的时候，从文档的第一行中提取的，因为文档的第一行的格式固定是 #加上文档的标题。
- txt : 原始文档的内容。这是最有价值的部分。
- kwd : key words 从该文档中提取的关键字信息。譬如：vacuum,3|hash index,2|WAL,5， 则表示该文档中vacuum这个关键字出现了3次，hash index这个关键字出现了2次，WAL这个关键字出现5次。
- ref : 本文档的参考文档，可能为空。

