# (1) コンパイラ
CC  = g++

# (2) コンパイルオプション
CFLAGS = -O3 -Wall

# (3) 実行ファイル名
TARGET = main

# (5) オブジェクトファイル名
OBJS = main.o alpha_beta.o evaluation.o game.o hash.o player.o time.o global.o

# (9) ターゲットファイル生成ルール
all: $(TARGET)

# (10) 実行ファイル生成
$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) -lm

# (11) オブジェクトファイル生成ルール
%.o: %.cpp
	$(CC) $(CFLAGS) -c $<

# (12) クリーンアップ
clean:
	rm -f $(TARGET) *.o
