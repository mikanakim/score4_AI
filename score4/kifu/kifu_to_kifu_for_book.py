import csv
import copy

# 棋譜を定石作成用のフォーマットに変換する
# 盤面の対称性を担保する

load_file_path = "./kifu/kifu.csv"
save_file_path = "./kifu/kifu_for_book.csv"

# csvファイルを読み込み
with open(load_file_path) as f:
    kifu = list(csv.reader(f))
    kk = copy.deepcopy(kifu)

# csvファイルをint化
for i in range(len(kifu)):
    # リストの要素を数値化（最終要素を除く）
    kifu[i] = [int(kifu[i][v]) for v in range(len(kifu[i]) - 1)]
    kifu[i].append(kk[i][-1])    
    # リストの最後の要素をチェックして整数化
    if kifu[i] and isinstance(kifu[i][-1], str) and kifu[i][-1].isdigit():
        kifu[i][-1] = int(kifu[i][-1])

new = []

for i in range(len(kifu)):
    if kifu[i][-1] == 'w':
        kifu[i][-1] = 1
        kifu[i].append(64-len(kifu[i]) + 1)
        new.append(kifu[i])
    elif kifu[i][-1] == 'd':
        kifu[i][-1] = 0
        kifu[i].append(0)
        new.append(kifu[i])
    elif kifu[i][-1] == 'l':
        kifu[i][-1] = -1
        kifu[i].append(64-len(kifu[i]) + 1)
        new.append(kifu[i])

with open(save_file_path, 'w') as f:
    writer = csv.writer(f)
    writer.writerows(new)

print('kifu has been converted to kifu_for_book!')


