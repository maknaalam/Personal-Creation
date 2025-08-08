#include <bits/stdc++.h>
using namespace std;

map<string, string> generateHealthBar(int current, int max, int width) {
    int filled = current * width / max;
    string bar = "<";
    for (int i = 0; i < width; i++) {
        if (i < filled)
            bar += "=";
        else
            bar += "-";
    }
    bar += ">";

    int percent = current * 100 / max;

    map<string, string> result;
    result["bar"] = bar;
    result["percent"] = to_string(percent);

    return result;
}


typedef struct gridnode_g {
    char value; // - = kosong, e = start, c = end, + = explored, o = path, # = obstacle
    bool visited;
    pair<long, long> parent; // koordinat asal untuk BFS trace back
    /*
        storing the parent id to reach the current coordinate that being the id key of the map:
        Lapisan	                Arti
        pair<long, long>	    koordinat (row, col)
        map<pair, pair>	        key = anak, value = orang tuanya (jalur BFS)

        parent[ {2,1} ] = {3,1};
        parent[ {1,1} ] = {2,1};
        parent[ {1,2} ] = {1,1};
        Artinya:
        - Kita sampai ke (2,1) dari (3,1)
        - Kita sampai ke (1,1) dari (2,1)
        - Kita sampai ke (1,2) dari (1,1)
        Maka, jika titik akhir kita ada di (1,2), jalurnya bisa ditelusuri mundur:
        (1,2) ← (1,1) ← (2,1) ← (3,1) ← START
        */
} GRIDnode;

struct Graph {
    GRIDnode* grid;
    /*
    2D View (row x col = 3x4):
    [ (0,0) (0,1) (0,2) (0,3) ]
    [ (1,0) (1,1) (1,2) (1,3) ]
    [ (2,0) (2,1) (2,2) (2,3) ]

    1D View:
    index = row * col + col
    grid[0*4 + 0] = grid[0]
    grid[0*4 + 1] = grid[1]
    ...
    grid[2*4 + 3] = grid[11]
    */
    long row, col;
    pair<long, long> start, end; // start = enemy, end = player
    queue<pair<long, long>> step;
    /*
    for knowing where the actual id is, let say:
    start = {3, 1};
    end = {2, 2};
    */

    void init(long r, long c) {
        row = r; // r = row
        col = c; // c = column
        grid = (GRIDnode*)malloc(sizeof(GRIDnode) * row * col);
        for (long i = 0; i < row * col; i++) {
            grid[i].value = '-';
            grid[i].visited = false;
        }
    }
    // why idx only? Because originally it is a 1D array, or you may say as a single row, which represents x-axis
    long idx(long r, long c) {
        return r * col + c;
    }

    void set_point(long r, long c, string cmd) {
        if (cmd == "start") {
            grid[idx(r, c)].value = 'e';
            start = {r, c};
        } else if (cmd == "end") {
            grid[idx(r, c)].value = 'c';
            end = {r, c};
        } else if (cmd == "obstacle") {
            grid[idx(r, c)].value = '#';
        }
    }

    void print() {
        for (long r = 0; r < row; r++) {
            for (long c = 0; c < col; c++) {
                cout << grid[idx(r, c)].value << " ";
            }
            cout << "\n";
        }
    }

    void print_hide() {
        for (long r = 0; r < row; r++) {
            for (long c = 0; c < col; c++) {
                if (grid[idx(r, c)].value == '+' || grid[idx(r, c)].value == 'o') cout << '-' << " ";
                else cout << grid[idx(r, c)].value << " ";
            }
            cout << "\n";
        }
    }
    // This checks whether the neighboring coordinate (nr, nc) is inside the bounds of the grid
    bool is_valid(long r, long c) {
        return r >= 0 && c >= 0 && r < row && c < col;
    }
    /*
    This is actually a crucial function!
    1. ✅ is_valid(r, c) prevents accessing cells outside the grid bounds — and this is correct and necessary.
    2. ❌ BFS doesn’t go to the “other side” of a wall, because you don’t allow crossing value == 1.
    3. ❗️Even if you’re printing the whole grid, the traversal is still limited by those two conditions.
    */

    void bfs() {
         while (!step.empty()) {
            step.pop();
        }
        queue< pair<long, long> > q;
        /*
        storing the coordinate that going to be processed in the bfs, let say:
        q = [ (3,1) ]
        */
        q.push(start);
        grid[idx(start.first, start.second)].visited = true;
        // delta row (direction change in rows)
        long dr[4] = {-1, 1, 0, 0}; // atas, bawah
        // dc = delta column (direction change in columns)
        long dc[4] = {0, 0, -1, 1}; // kiri, kanan

        bool found = false; // to check wether it finishes reaching the end

        while (!q.empty() && !found) {
            int level_size = q.size(); // 🔹 process one BFS level at a time
            for (int i = 0; i < level_size; i++) {
                auto current = q.front(); q.pop();
                long r = current.first;
                long c = current.second;

                for (int d = 0; d < 4; d++) {
                    long nr = r + dr[d]; // nr = new row
                    long nc = c + dc[d]; // nc = new column

                    if (is_valid(nr, nc)) {
                        // This gets a reference to the grid node at coordinate (nr, nc), so that we can access the node directly
                        GRIDnode &next = grid[idx(nr, nc)];
                        if (!next.visited) {
                            next.visited = true;
                            if (next.value == '#') // if facing an obstacle
                                continue;

                            next.parent = {r, c};
                            q.push({nr, nc});

                            if (next.value != 'c') // if visited, but not the end
                                next.value = '+';

                            if (nr == end.first && nc == end.second)
                                found = true; // found, but finish this level also
                        }
                    }
                }
            }
        }
        // Backtracking the path to reconstruct the shortest path
        if (found) {
            vector<pair<long, long>> tmp;
            pair<long, long> cur = end, prev;
            tmp.push_back(cur);
            while (cur != start) {
                prev = grid[idx(cur.first, cur.second)].parent;
                if (prev == start) break;  // ← stop before setting to self
                tmp.push_back(prev);
                if (grid[idx(prev.first, prev.second)].value != 'e')
                {
                    grid[idx(prev.first, prev.second)].value = 'o';
                }
                
                cur = prev;
            }
            // reverse to make front = next move from enemy
            reverse(tmp.begin(), tmp.end());
            for (auto &p : tmp) step.push(p);
        }
    }

    void reset_bfs_state() {
        for (long r = 0; r < row; r++) {
            for (long c = 0; c < col; c++) {
                GRIDnode &cell = grid[idx(r, c)];
                if (cell.visited == true)
                {
                cell.visited = false;
                cell.parent = {-1, -1};
                if (cell.value == '+' || cell.value == 'o') {
                    cell.value = '-'; // clear path or visited marker
                }
                }

            }
        }
    }

    void movement(char user,char key) {
        long nr, nc;
        if (user == 'c') {
            nr = end.first;
            nc = end.second;
        } else if (user == 'e') {
            nr = start.first;
            nc = start.second;
        } else {
            cout << "Invalid user\n";
            return;
        }
        GRIDnode &now = grid[idx(nr, nc)];
        if (key == 'w')
        {
            if (is_valid(nr-1, nc) && grid[idx(nr-1, nc)].value != '#')
            {
                if (user == 'c') {
                    if (grid[idx(end.first, end.second)].value != 'e')
                    {
                        grid[idx(end.first, end.second)].value = '-';
                    }
                    end = {nr-1, nc};
                } else if (user == 'e') {
                    if (grid[idx(start.first, start.second)].value != 'c')
                    {
                        grid[idx(start.first, start.second)].value = '+';
                    }
                    start = {nr-1, nc};
                }
                grid[idx(nr-1, nc)].value = user;
            }
        } else if (key == 'a')
        {
            if (is_valid(nr, nc-1) && grid[idx(nr, nc-1)].value != '#')
            {
                if (user == 'c') {
                    if (grid[idx(end.first, end.second)].value != 'e')
                    {
                        grid[idx(end.first, end.second)].value = '-';
                    }
                    end = {nr, nc-1};
                } else if (user == 'e') {
                    if (grid[idx(start.first, start.second)].value != 'c')
                    {
                        grid[idx(start.first, start.second)].value = '+';
                    }
                    start = {nr, nc-1};
                }
                grid[idx(nr, nc-1)].value = user;
            }
        } else if (key == 's')
        {
            if (is_valid(nr+1, nc) && grid[idx(nr+1, nc)].value != '#')
            {
                if (user == 'c') {
                    if (grid[idx(end.first, end.second)].value != 'e')
                    {
                        grid[idx(end.first, end.second)].value = '-';
                    }
                    end = {nr+1, nc};
                } else if (user == 'e') {
                    if (grid[idx(start.first, start.second)].value != 'c')
                    {
                        grid[idx(start.first, start.second)].value = '+';
                    }
                    start = {nr+1, nc};
                }
                grid[idx(nr+1, nc)].value = user;
            }
        } else if (key == 'd')
        {
            if (is_valid(nr, nc+1) && grid[idx(nr, nc+1)].value != '#')
            {
                if (user == 'c') {
                    if (grid[idx(end.first, end.second)].value != 'e')
                    {
                        grid[idx(end.first, end.second)].value = '-';
                    }
                    end = {nr, nc+1};
                } else if (user == 'e') { 
                    if (grid[idx(start.first, start.second)].value != 'c')
                    {
                        grid[idx(start.first, start.second)].value = '+';
                    }
                    start = {nr, nc+1};
                }
                grid[idx(nr, nc+1)].value = user;
            }
        }
    }

};

int main() {
    // Seed the random number generator with current time
    srand(time(0));

    Graph g;
    g.init(10, 10);
    g.set_point(6, 0, "start");
    g.set_point(3, 5, "end");
    g.set_point(6, 2, "obstacle");
    g.set_point(5, 1, "obstacle");

    // cout << "Initial grid:\n";
    // g.print();

    g.bfs();

    // cout << "\nAfter BFS:\n";
    // g.print();
    // puts("\n");
    g.print_hide();


    bool trace_path = true, invalid = false;
    while (trace_path)
    {
        if (invalid)
            cout << "invalid input\n";

        cout << "Use w/a/s/d to move, or write p to detect bot path (-1 move chance)\n";
        char moveset;
        cin >> moveset;
        if (moveset != 'w' && moveset != 'a' && moveset != 's' && moveset != 'd' && moveset != 'p')
        {
            continue;
        } else if (moveset == 'p') {
            g.print();
            puts("\n");
            g.print_hide();
        } else {
            g.movement('c',moveset);
            g.print_hide();
            g.reset_bfs_state();
            g.bfs();
        }
        
        if (g.start == g.end) {
            cout << "Miku Encountered! ";
            trace_path = false;
        }
        else {
            pair<long, long> path=g.step.front();
            
            long r_e = path.first - g.start.first;
            long c_e = path.second - g.start.second;

            char moves;
            if (r_e == -1 && c_e == 0)
                moves = 'w';
            else if (r_e == 0 && c_e == -1)
                moves = 'a';
            else if (r_e == 1 && c_e == 0)
                moves = 's';
            else if (r_e == 0 && c_e == 1)
                moves = 'd';
            else{
                if (!g.step.empty()) {
                    pair<long, long> invalid_move = g.step.front();
                    cout << "[BOT] no valid move! step invalid: (" << invalid_move.first << "," << invalid_move.second << ")\n";
                } else {
                    cout << "[BOT] no valid move! step queue is empty\n";
                }
                continue;
            }

            // Generate a random number between 1 and 100
            int randomNumber = rand() % 100;
            cout << randomNumber << "\n";
            if (randomNumber % 2 == 0 || randomNumber % 5 == 0)
            {
                g.movement('e',moves);
                g.step.pop();
            }
        }
            
    }
    free(g.grid);

    const int e_maxHealth = 180, p_maxHealth = 120, barWidth = 60;
    int e_currentHealth = 180, p_currentHealth = 120, e_win=0, p_win=0, err=0, p_skill=0, p_attack=0, e_skill=0, e_energy=100, p_energy=100;
    // This line runs the "chcp 65001" command
    // and sets the terminal to the correct UTF-8 character set.
    system("chcp 65001");
    string ascii_art_1 = R"(
    爪 讠长 ㄩ

          /＾>》, -―‐‐＜ ＾}
        ./:::::/,≠´:::::;::ヽ.
        /:::::〃:::::::／}::丿ハ
      ./:::::::i{:::／　ﾉ／ }::} 　.ᐟ.ᐟ)";
    string ascii_art_2_win = R"(      /::::::::瓜イ⸝⸝O　 ´ O,'ﾉ　< woah I WIN!!111!! ))";
    string ascii_art_2_lost = R"(      /::::::::瓜イ⸝⸝O　 ´ O,'ﾉ　< woah I lost I guest ... ))";
    string ascii_art_3 = R"(    ./:::::::::|ﾉﾍ.{､　 ヮ_.ノﾉイ
    |::::::::::| ／,}｀ｽ/￣￣￣￣/
    |::::::::::|(_:::つ/　  　 /
          ￣￣￣￣￣＼/＿＿＿＿/)";
    
    string ascii_art2 = R"(
　　　　　　　　　　　　　　 　　.　　-‐…‐-　 ..,_　　 　　____
　　　　　 　 　 　 　  　  i㌻'´　　　　　　　 ｀丶、 /:/:∧
　　　　　　　　　　 　   　 　 　 　 i 　 　 .　.　../:/:/　 __
　　　　　　　　　  　  /　　　ｉ￤ . . ｉ ￤. . . .:____厶=-‐=＝=‐-　 _
　 　 　 　 　 　  　 / 　 　 .|｜!. . .|　|.｜.i　 : Ｖﾑ=- 　‐…‐-　　.,_｀丶
　　　　　　　　 　  ′　　ｉ :|｜i . :｜ |_｣__|. ｉ . : Vﾑ　..,_　　　｀丶::.
.　　　 　 　 　  　 ;　　　 i:Τ|八. :｜ |.ﾉ　|.八 . j.￤Ｖﾑ//｀ヽ、　 　::、 _,,..　　　　　..._＿____
　　　　　　　　　   {八 　:j灯心　＼|　'て]心Y∨. : . |ｉ.　 ｀、＼　　　　__ﾆ:、　_,,.. 　-‐=＝冖冖＝=‐- 　..,,_
.　　　 　 　 　 　  { i个ト､ vツ ,　　 　 V^ツﾉ/ . . ｉ |　　　、 ｀　.,,__,,..ﾆ=-‐、^´‐-=ﾆ..,,___ -‐=＝＝=‐-　 ｀` : .
　　　　 　 　 　  　  И:.:i ' '　　　　 　 　 ' ' / ,:　}　	   ｀ﾆ=-‐　　　　￣＼ 　 　 　 　 　 　 ー‐…‐-..,,_ 　｀ヽ
　　 　 　 　 　 　  　 | i八　　 ⅴ　 ,　　 　 イj／　 ＾ﾝ　　　　　　　　　　　　 　 　 ＼　　　　　　　 　 　 　 　 　 ー- ミiト､ :.
　　　　　　　　　 　 ｛｀＾ﾆ辷ｭ　　 ´　　　_.イ｀¨ﾆ¨´＼　　　　　　　　　　 　 　 　 　 　 　 丶、　　　　　　 　 　 　 　 　 　 　:,:､
　　　　　 　 　 　 　 　 　 　 `  __,,.　´ |,ﾉ　　 ＼ : .｀: 、　　　　　　　　　　　　　　　 　｀..,,___,,..　　　　　　　　　　　}　＼
　　　　　　　　　　　　　　_　 ｧ'´|　　 　 ／^:､_ 　 　 　: . :｀　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　 　 ′　　 ,
　　　　　　　　　　　　　;´　/　{.｣_,,..　　/厶ﾆ.._　　　 . : . 　: .､　　　　　　　　　　　　　　　　　　　　　　　　 　 　 　	   ／　　 ｝
　　　　　　　　　　　 　 {　厶／ 7|　rxｸ ／/ 　 　 ｀ 　　＼ . : __,,.｀ﾆ=--‐…‐-　 _　　　　　 　 　 　 　 Л　　　　　 　 　   ノ
　 　 　 　 　 　 　 　 　 ∨　|__,/｜__,／　/　　 　 　 　 ｀`￢=;::r‐-=ﾆ二　 ..,,__‐‐-｀ミメ､　　　 　 　/／
　　　　　　 　 　 　 　 . '′/ ∥ 〉　　　　　′　　　　　　　　　　} }.::.::.::.::.:｀^Ｔi冖 ｒ‐--‐ _…―--‐　 ∠二つ
　　　　　 　 　 　 　 /　　/:::;′rｬｾ冖 ｛　　 　 ..,,___　　　　_ﾉｿ.::.::.::.::.::.::.L上⊥!＋┴｀ヽヽ　_,／⌒´
.　　　　　　 　 　 　 { 　 :::/　　 …‐┘.: . : : /::::....._⌒'く.::.::.::.::.::.::.::.::.::.::.::｝｝ {
　　　　　　　　 　 　 :,　ｉ::| 　 　 　 . . ､　/:.::.::.::.｀^‐-｀ﾆ=‐-_　　..,,_.::.::.::.::.::.｝ﾄ､＼
　　　　　　　　　　 　 ＼|: i |　　　. . . : ＼/　＼.::.::.::.::.::.::.::.::.::.::.::.:..｀ヽ 　ノﾝ　　┘
　　　　　　　　　　　　　|: i |　　　 　  　  ＼]_　　＼.::.::.::.::.:::.::.}／　 　 ｀　‐---ヤ^´
.　　　　　　　 　 　 　  |:::|　　　 　 　　 　｛　　 　 丶、.::.::.::::.::;ﾉ　　　　　　　　　
　　　 　 　 　 　 　 　  |ii||　　　　　　　　 　:、 　 　 　 ｀　‐--‐ '^´　　　　　　　　 　
　　　　　　　　　　　　　｛ :｛ 　 　 　 　 　 　 　 ＼　　
    )";
    string ascii_art3 = R"(
　　　　　　　　　　　　　　　　　,《///《≫　´￣￣ヽミ~ .   《//》ｘ
　　　　　　　　　　　　　 　 　 ,《//≫´ 　　　　　　 ー=ミ \《//》'⌒丶　 　 　 　 (⌒ヽ
　　　　　　 　 　 　 　 　 .　≪》'／　　　　　 　 　＼ー=ミ《//》'　  ＼　　　　 　） 　⌒)
.　　　　　　　 　 　 　  '／　 //〃　/　￤　　i　 、　＼ｰ=《//,《 　 　 ＼　　　 （￣￣
　　　　　　　　　 　  ／／　　//,/ 　 ′_｣j_　 | 　 ＼　＼/》//》　 　 　 ’ ,　 　 ）
　　　　　　　　　　 　 　 　 /ｲ/'　ｉ '/八　　| ＼ ⌒＼　　∨/l《∧ 　 　  : : ’，
　 　 　 　 　 　  / / 　 　/: :|/　│,__　＼　|　 ＼ ＼　i|'/|≫ﾍ　 　      : :‘
　　　　　　　　  ′　　　　   : /｝  j {´クﾊ　＼|　　_,.＼　 ｉい|'/| ，     　 : : ，
　　 　 　 　 　 { 〃　　　:::　 { |j八弋::ﾉ　　　´⌒~ ﾘゝ|Ｎi|'/}　 　‘，　    : : :’
　　 　 　 　 　  ∨　 　 :::/ 　 ‘,i ﾊゝ　　　'　　　 /八|_,｣ﾚ'′ 　     ， 　  　 : : }；
　　　　　　　　　　　 　:::′　    ‘八 人　　 {ヽｰ 　'//　/¨ﾘ　　　	 }　　 : : : :八
　 　 　 　 　 　′ 　 :::　 　  　 ＼{ﾍ: :个: | }　 イ/:://　　　　   　′ 　  : :/　 ’　 ｝
　　　　　　　　j　 　 :::{ 　  　 　 　＼い i| { /^｝|{::ｲ＿_,,..- ､/ 　　 　   : :′　,
　　　　　　　　！ 　  ::ﾘ　 　 　  ''¨￣ﾊ   Ⅵ  ｀勹 イ乂 |厂|}　　　∨ 　 　 　  /　 /
　　 　 　 　 　‘　　:::八　　 　　/　 　/八/ |　 二}/八   !} |}　　 /　　　　: :/ （
　　　　　　　　 ‘,　::::′ 、　　 /　　 /　{厶|　 しｲi　∧ i|]　|}　　　　　 : :/
　　　 　 　 　    : ::{::::＼　/　　 /　　//}　　八∨　  Ⅵ}　|}　 ′　　　: :′
　 　 　 　 　 　 　 ＼い:::::  ∨'⌒∨　／{{/ 　./＼＼　　 )　_乂}　/　/ : : ／
　　　　 　 　 　 　 　 ヽ＼::: Ｖ⌒7／:::八　  /::＼ヽ　 　　 Ｙ/: :/ : :〃
　　　　　　　　 　 　 　 ）:: ｝:::｛::::::∨:::::: ｝｝　　}{: :｛ : :/{{
    )";
        string ascii_art4 = R"(
　　　　　　　　　　　　　　　　　　　　　　　　　　 　　　　  ）
　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　ヽ　異議あり！！
　　　　　　　　　　　　　　　　　　 ,ﾍ　　　　　　　 　.へ　　　 ）
　　　　　　　　　　　　　　　　　,.'　 〉=γ、 ≫'´V` ､'　　 ＼　`ｖ'⌒ヽ／⌒ヽ／__ ,. ‐-.. _
　　　　　　　　　　　 　 　 .　'´ 　  //〃／/  , ､　 ＼､. 　  ＼　　　　　　 _,ィ７／　__｀` ｰ- 、
　　　　　　　 　 　 　 ,　'´　　　　// ｒ’ /  /　 ＼  ＼::　  ヽ〉 ＼　_ｘ==////　　　ゝヽ￣ヽ　ｰ- '
　　　　　 　 　 　 ,. '´　　　　　/ /´》i/  /⌒\ ､  ﾏ      ∧`.トヾ::　.ｘ＜///////!　、_ヽ'ﾉ_.〉
　　 　 　 　 　 ／　 　 　 　 　./ {ﾟ_}| ﾄﾚｲ ιﾉ ` V ,_ﾄ､ |ﾐ-ゝ ,ｘ＜//////////|ｘ｀＞､..ﾉ=┘
　　 　 　 　／　　　　　　 　::/　　 ヾｔ λﾄ　　　､  ιﾘλ!＞升////////////∧////|
　　　　　／ 　 　 　 　　 :::/　　　　　 ｘ`  (￣) νィ´//////////////////X,//|
　　　　／ 　 　 　　 　 :::/　　　　　　　　ﾉ` ､ 　彳'´　V///////ZZ≫=――===''''''´ ＞ ､
　 　, '　　　　　　　::::/　　　　　　 _i＜_ｒ'￣ｲ　 　.:У´￣￣ ､::::　　　　　　　　＞ ､
　　/　　　　　　　　::::/ 　 　 　　 '",. |,ｲ_λ/ | 　::／　　　　＼:::::::: 　　　　　　 ＞ ､
　/　　　　　　　　:::::/　　　　   /,.ィ　/　|　==Ｖ　/　　　　　　＼:::::::::::::　　　　　　 ` ､
)";
        string ascii_art5 = R"(
　　　　　　　  ┌──────────────┐
　　　　　　　  |　　 　 　    .|
　　　　　　　　 |　 　　　 　   |
　　　　　　 ﾍ　,-‐‐-ｧﾍ　　　　  |
　　　 　ｧ'´　》:::::ト、ヽ |────┘
. 　　　/ ::彳rｸ／ } |　＼ﾍ＼
　 　 ∧  :::匸） ┃ }ﾉ ┃ {／.|
　　 ∧  ::: ゞ弋｀｀ ｀人 | |
　　 | ::::|　 ﾚ个ーヤ く  | |
　　 | ::: |　　/ ,不、__〃｀ヽ
　　 |:::.|　 ,' ﾒ ┤  ﾊ　乂;;;}
　　 |:::|　／ ７|  | |｀ヽ､_ノ
　　ｊ::|  ／_/;;|  |=|:    | |
-=彡 ::|　＾⌒ゝ;匕 ｲヽｲ     | |
::::::.|　　　　ﾄ 7 ﾄ 7:＼   | |
::::: ﾉ　　　　 ﾚ′ ｌ人:::彡' | | 
)";
    while (e_win==0 && p_win==0)
    {
        if (err == 1)
        {
            cout << "Invalid input\n";
            err = 0;
        } else if (err == 2) 
        {
            cout << "Insufficient energy\n";
            err = 0;
        }
        if (p_skill == 1)
        {
            p_attack = 3;
            p_skill = 0;
        } else
        {
            cout << "player health is: " << p_currentHealth << "\n";
            cout << "player energy is: " << p_energy << "\n";
            cout <<
                R"(
Choose One of this option:
1. Smash -- 0 energy (30 damage)
2. Delayed punch -- 30 energy (20 damage this turn, 65 damage next turn. Skip next turn)
3. Heal -- 50 energy (gain 80 health)
4. Dodge -- 0 energy
5. Regain energy (70 energy)
= )";
            int a;
            cin >> a;
            if (a == 1)
            {
                p_attack = 1;
            } else if (a == 2) {
                if (p_energy < 30)
                {
                    err = 2;
                    continue;
                }
                p_attack = 2;
                p_skill = 1;
                p_energy -= 30;
            } else if (a == 3)
            {
                if (p_energy < 50)
                {
                    err = 2;
                    continue;
                }
                p_currentHealth += 80;
                p_energy -= 50;
                if (p_maxHealth < p_currentHealth)
                {
                    p_currentHealth = 120;
                }
            } else if (a == 4) {
                p_skill = 2;
            } else if (a == 5) {
                p_energy += 70;
                if (100 < p_energy)
                {
                    p_energy = 100;
                }
            } 
            else
            {
                err = 1;
                continue;
            }
        }
        
        // Generate a random number between 1 and 100
        int randomNumber = rand() % 100;
        cout << randomNumber << "\n";
        if (e_currentHealth / 60 < 1)
        {
            if (randomNumber % 7 == 0 || randomNumber % 5 == 0 || randomNumber % 3 == 0 ) {
                if (e_energy < 40)
                {
                    e_skill = 1;
                } else
                {
                    e_skill = 2;
                    e_energy -= 40;
                }
            } else if (randomNumber % 8 == 0)  // dodge
            {
                p_attack = 0;
                cout << ascii_art2 << "\n";
                cout << "Miku choose to dodge\n";
                continue;
            } else 
            {
                e_skill = 3;
            }
        } else {
            if (randomNumber % 7 == 0 || randomNumber % 6 == 0) 
            {
                if (e_energy < 30)
                {
                    e_skill = 1;
                } else
                {
                    e_skill = 4;
                    e_energy -= 30;
                }
            } else if (randomNumber % 5 == 0) 
            {
                if (e_energy < 40)
                {
                    e_skill = 1;
                } else
                {
                    e_skill = 2;
                    e_energy -= 40;
                }
            } else if (randomNumber % 8 == 0) { // dodge
                p_attack = 0;
                cout << ascii_art2 << "\n";
                cout << "Miku choose to dodge\n";
                continue;
            } else {
                e_skill = 3;
            }
        }

        if (p_attack == 1)
        {
            e_currentHealth -= 30;
            p_attack = 0;
            if (e_currentHealth <= 0)
            {
                p_win = 1;
            }
        } if (p_attack == 2)
        {
            e_currentHealth -= 20;
            p_attack = 0;
            if (e_currentHealth <= 0)
            {
                p_win = 1;
            }
        } if (p_attack == 3)
        {
            cout << "Second hit!!\n";
            e_currentHealth -= 65;
            p_attack = 0;
            if (e_currentHealth <= 0)
            {
                p_win = 1;
            }
        }


        if (p_win == 1)
        {
            continue;
        }
        
        
        if (e_skill == 1)
        {
            cout << ascii_art3 << "\n";
            cout << "Miku regain energy\n";
            e_energy += 70;
            if (100 < p_energy)
            {
                e_energy = 100;
            }
            e_skill = 0;
        } else if (e_skill == 2)
        {
            cout << ascii_art3 << "\n";
            cout << "Miku regain health\n";
            e_currentHealth += 60;
            if (e_maxHealth < e_currentHealth)
            {
                e_currentHealth = 180;
            }
            e_skill = 0;
        } else if (e_skill == 3)
        {
            cout << ascii_art5 << "\n";
            cout << "Miku use basic attack\n";
            if (p_skill == 2)
            {
                cout << "But you dodge it\n";
                p_skill = 0;
            } else {
                p_currentHealth -= 30;
                e_skill = 0;
                if (p_currentHealth <= 0) {
                e_win = 1;
                }
            }
        } else if (e_skill == 4)
        {
            cout << ascii_art4 << "\n";
            cout << "Miku use ultimate attack\n";
            p_currentHealth -= 60;
            e_skill = 0;
            if (p_currentHealth <= 0) {
            e_win = 1;
            }
        }
        if (e_win == 1)
        {
            continue;
        }

        cout << "Miku\n";
        map<string, string> healthData2 = generateHealthBar(e_currentHealth, e_maxHealth, barWidth);
        cout << healthData2["bar"] << " " << healthData2["percent"] << "%\n";
        cout << "Player\n";
        map<string, string> healthData = generateHealthBar(p_currentHealth, p_maxHealth, barWidth);
        cout << healthData["bar"] << " " << healthData["percent"] << "%\n";
    }
    if (p_win)
    {
        cout << ascii_art_1 << "\n";
        cout << ascii_art_2_lost << "\n";
        cout << ascii_art_3 << "\n";
        cout << "player win";
    } else {
        cout << ascii_art_1 << "\n";
        cout << ascii_art_2_win << "\n";
        cout << ascii_art_3 << "\n";
        cout << "player lost";
    }
    return 0;
}

