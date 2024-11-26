import copy
import game

TEBAN = {0:'s', 1:'g', -1:'error'}
def find_subvector_id(target_subvector, lst):
    for i, subvector in enumerate(lst):
        if subvector == target_subvector:
            if subvector[-1] == 'w':
                return str(i) + '_win'
            elif subvector[-1] == 'l':
                return str(i) + '_lose'
            elif subvector[-1] == 'd':
                return str(i) + '_draw'
            else:
                return str(i)
    return None

def check(a, b):
    if len(a) > len(b):
        return False
    count = 0
    for i in range(len(a)):
        if a[i] == b[i]:
            count += 1
    if count == len(a):
        return True
    else:
        return False

def find_subsets(element, lst):
    subsets = [x for x in lst if check(element, x)]
    for subset in subsets:
        lst.remove(subset)
    return subsets

def q_notation(lst, lst_const, win_lose):# win_loseは内部処理で'w'とか'l'とかを一旦外すためのもの。初期値は空リストを入力
    result = []
    s_or_g = 0
    while lst:
        element = lst.pop(0)
        if element[-1] == 'w' or element[-1] == 'd' or element[-1] == 'l':
            win_lose.append(element[-1])
            del element[-1]
        else:
            win_lose.append(None)
        subsets = find_subsets(element, lst)
        if subsets:
            if win_lose[-1] != None:
                element.append(win_lose[-1])
                del win_lose[-1]
            else:
                del win_lose[-1]
            result.extend([find_subvector_id(element, lst_const), q_notation(subsets, lst_const, win_lose)])
        else:
            if win_lose[-1] != None:
                element.append(win_lose[-1])
                del win_lose[-1]
            else:
                del win_lose[-1]
            result.append(find_subvector_id(element, lst_const))
    return result

def max_depth(lst):
    if not isinstance(lst, list):  # リストでない場合、深さは0を返す
        return 0
    elif len(lst) == 0:  # 空のリストの場合、深さは1を返す
        return 1
    else:
        depths = [max_depth(item) for item in lst]  # リスト内の各要素の深さを再帰的に計算
        return 1 + max(depths)  # 1を加えて最大の深さを返す

def is_one_dimensional(lst):
    for item in lst:
        if isinstance(item, list):
            return False
    return True

def print_tree(tree, max_depth, kifu, level=0, same_layer=0, bar=[], s_or_g = [0], prefix1="├───── ", prefix = "└───── ", prefix3 = "|      "):
    for i in range(len(tree)-1):
        if not isinstance(tree[i+1], list):
            same_layer += 1

    for idx, item in enumerate(tree):
        if idx != 0 and idx != len(tree)-1:
            if not isinstance(tree[idx-1], list) and not isinstance(tree[idx+1], list):
                if not is_one_dimensional(tree):
                    bar.append(level)
        if isinstance(item, list):#listの場合
            tmp = copy.deepcopy(tree[idx-1])
            wdl = 0
            if '_win' in tmp:
                tmp = tmp.replace('_win', '')
                wdl += 1
            elif '_lose' in tmp:
                tmp = tmp.replace('_lose', '')
                wdl += 1
            elif '_draw' in tmp:
                tmp = tmp.replace('_draw', '')
                wdl += 1
            tmp2 = copy.deepcopy(kifu[int(tmp)])
            if wdl > 0:
                del tmp2[-1]
                wdl = 0
            s_or_g.append(len(tmp2)%2)#直前の石の総数が偶数なら次は先手
            print_tree(item, max_depth, kifu, level + 1, same_layer, bar, s_or_g, prefix)
            if len(bar) > 0:
                del bar[-1]
            if len(s_or_g) > 1:
                del s_or_g[-1]
        else:
            if same_layer > 1:
                prefix = "├───── "
                same_layer += -1
            bar = sorted(list(set(bar)))
            c = 0
            indentation = ''
            #どちらの手番か示すフラグ
            flag = s_or_g[-1]
            if len(bar) > 0:
                for i in range(bar[-1] + 1):
                    if i <= bar[-1]:
                        if i not in bar and level != 0:
                            indentation += "       "
                            c += 1
                        elif i in bar and level != 0:
                            indentation += "|      "
                            c += 1
                indentation += "       " * (level - c)

            else:
                indentation = "       " * level
            
            #wdlの記載がある場合
            wdl = ['', '_win', '_lose', '_draw']
            wdl_flag = 0
            for i in range(1, len(wdl)):
                if wdl[i] in item:
                    wdl_flag = i
                    item = item.replace(wdl[i], '')
            print(f"{indentation}{prefix}{item}{TEBAN[flag]}{wdl[wdl_flag]}")
            prefix = "└───── "

def remove_duplicates_2d_list(lst):
    flattened_list = [item for sublist in lst for item in sublist]
    unique_items = list(set(flattened_list))
    result = [[item for item in sublist if item in unique_items] for sublist in lst]
    return result

def is_two_numbers(string):
    try:
        n, m = map(int, string.split())
        return True
    except ValueError:
        return False

def extract_common_elements(x, y):
    common_elements = []
    idx = 0
    for i in range(min(len(x), len(y))):
        if x[i] == y[i]:
            common_elements.append(x[i])
            idx  =  i
        else:
            break
    return common_elements, idx

def display_state(state, lst):
    if 'w' in lst or 'd' in lst or 'l' in lst:
        for idx in range(len(lst)-1):
            state = state.next(lst[idx])
    else:
        for idx in lst:
            state = state.next(idx)

    print(state, end = '\n')

def cpp_convert(state, lst):
    lst_2 = copy.deepcopy(lst)
    if 'w' in lst or 'd' in lst or 'l' in lst:
        del lst_2[-1]
        print('size = ', len(lst_2))
        print(' '.join(map(str, lst_2)))
        print()
    else:
        print('size = ', len(lst_2))
        print(' '.join(map(str, lst)))
    if 'w' in lst or 'd' in lst or 'l' in lst:
        tmp = copy.deepcopy(lst)
        del tmp[-1]
    else:
        tmp = copy.deepcopy(lst)
    print('[', end = '')
    for i, element in enumerate(tmp):
        if i != len(tmp) - 1:
            print(state.inverse_convert(element), ', ', end = '')
        else:
            print(state.inverse_convert(element), end = '')
    print(']', end = '\n')
    print()

def replay(replay_state, lst):
    i = 0
    while i < len(lst)-1:
        print('「returnキー」: 次の手')
        print('「b」: １手戻る')
        print('「end」: 終了')
        letter = input()
        if letter == '':
            replay_state = replay_state.next(lst[i])
            print(replay_state, end = '\n')
            print('{}手目: {}\n'.format(i+1, replay_state.inverse_convert(lst[i])))
            cpp_convert(replay_state, lst[0:i+1])
            i += 1
        elif letter == 'b':
            if i == 0:
                print('1手目です。')
            else:
                replay_state = game.State()
                for k in range(i-1):
                    replay_state = replay_state.next(lst[k])
                print(replay_state, end = '\n')
                i -= 1
                cpp_convert(replay_state, lst[0:i])
                
        elif letter == 'end':
            break

def cpp_convert(state, lst):
    lst_2 = copy.deepcopy(lst)
    if 'w' in lst or 'd' in lst or 'l' in lst:
        del lst_2[-1]
        print('size = ', len(lst_2))
        print(' '.join(map(str, lst_2)))
        print()
    else:
        print('size = ', len(lst_2))
        print(' '.join(map(str, lst)))
    if 'w' in lst or 'd' in lst or 'l' in lst:
        tmp = copy.deepcopy(lst)
        del tmp[-1]
    else:
        tmp = copy.deepcopy(lst)
    print('[', end = '')
    for i, element in enumerate(tmp):
        if i != len(tmp) - 1:
            print(state.inverse_convert(element), ', ', end = '')
        else:
            print(state.inverse_convert(element), end = '')
    print(']', end = '\n')
    print()

def give_wdl(lst):
    if 'w' in lst or 'd' in lst or 'l' in lst:
        print('すでに勝敗が記載されています。', end = '\n\n')
    else:
        while True:
            print('「w/d/l」: 勝敗を入力してください')
            print('「back」: 戻る')
            letter = input()
            print()
            if letter == 'w' or letter == 'd' or letter == 'l':
                lst.append(str(letter))
                break
            elif letter == 'back':
                break

def correct_wdl(lst):
    if 'w' in lst or 'd' in lst or 'l' in lst:
        while True:
            print('「w/d/l, remove」: 修正後の勝敗を入力してください')
            print('「back」: 戻る')
            letter = input()
            print()
            if letter == 'w' or letter == 'd' or letter == 'l':
                lst[-1] = letter
                break
            elif letter == 'remove':
                del lst[-1]
                break
            elif letter == 'back':
                break
    else:
        print('勝敗が記録されていません。', end = '\n\n')