#include <iostream> 
#include <utility> 
#include <algorithm> 
#include <vector> 
#include <tuple>

#define Size 150 
#define MAXGEN 80 

static bool DEBUG = false;

using namespace std;
class catalyist;

class catinput
{
public:
	vector<tuple<int, int, int>> prevBlock;
	vector<tuple<int, int, int>> curBlock;
	int numcat;
	int firstgen;
	vector<vector<int>> init;

	vector<tuple<int, int, int>> validations;

};


vector<vector<int>> flipx(const vector<vector<int>>& c)
{
	vector<vector<int>> res = c;

	for (int i = 0; i < c.size(); i++)
		reverse(res[i].begin(), res[i].end());

	return res;
}

vector<vector<int>> flipy(const vector<vector<int>>& c)
{
	vector<vector<int>> res = c;

	reverse(res.begin(), res.end());

	return res;
}

vector<vector<int>> flipxy(const vector<vector<int>>& c)
{
	return flipy(flipx(c));
}

vector<vector<int>> flip(const vector<vector<int>>& c, int op)
{
	if (op == 0)
		return c;
	else if (op == 1)
		return flipx(c);
	else if (op == 2)
		return flipy(c);
	else if (op == 3)
		return flipxy(c);
}

void print(const vector<vector<int>> vals)
{
	cout << "\n";
	for (int y = 0; y < vals.size(); y++)
	{
		for (int x = 0; x < vals[0].size(); x++)
		{
			if (vals[y][x] % 4 == 0)
				cout << ".";
			else if (vals[y][x] == 1)
				cout << "O";
			else
				cout << "O";
				//cout << "O" << (vals[y][x] - 1) / 4;
		}
		cout << "\n";
	}
	cout << "\n";
}
class catalator
{
public:
	vector<vector<vector<int>>> vals;
	vector<vector<vector<int>>> gen_start;

	vector<vector<pair<int, int> > > pertrubations;
	
	vector<tuple<int, int, int>> static_validator; 
	vector<pair<int, int>> last_validator;
	vector<pair<int, int>> target_validator;
	int last_violations; 
	int max_validation_gen; 

	vector<int>  box; 
	vector<vector<int>>  genbox;

	int max_gen; 
	int firstactivegen;
	vector<vector<vector<int>>> glider_traces;
	int dx_init, dy_init;

	catalator() {

		if (glider_traces.size() == 0)
		{
			vector<vector<int>> gld{ {4,8,8,8,8,4,4,0},{4,4,8,8,8,8,4,4},{0,4,4,8,8,8,8,4},{0,0,4,4,8,8,8,8},{0,0,0,4,4,8,8,8},{0,0,0,0,4,4,8,8},{0,0,0,0,0,4,4,8},{0,0,0,0,0,0,4,4} };

			for (int i = 0; i < 4; i++)
				glider_traces.push_back(flip(gld, i));

		}

		vals = vector<vector<vector<int>>>(MAXGEN, vector<vector<int>>(Size, vector<int>(Size, 0)));
		gen_start = vector<vector<vector<int>>>(3, vector<vector<int>>(Size, vector<int>(Size, MAXGEN)));
		pertrubations = vector<vector<pair<int, int> > >(MAXGEN, vector<pair<int, int> >());
		genbox = vector<vector<int>>(MAXGEN, vector <int>(4, 0));
		max_validation_gen = 0; 

	}

	void init(const vector<vector<int>>& a, int firstgen = 14)
	{
		firstactivegen = firstgen;

		setup0(a);
		evolveall();
	}

	int max_validation()
	{
		int mg = -1; 

		for (auto i : static_validator)
		{
			mg = max(get<2>(i) / 4, mg); 
		}

		return mg; 
	}

	void setup_validations(const vector<tuple<int, int, int>>& v)
	{
		for (auto i : v)
			static_validator.push_back(make_tuple(get<0>(i) + dx_init, get<1>(i) + dy_init, get<2>(i)));

		max_validation_gen = max_validation();
	}

	bool validate(int gen, const vector<tuple<int, int, int>>& v)
	{
		for (auto i : v)
		{
			int v = get<2>(i); 
			int expected = v % 2; 
			v /= 2; 
			bool onlyeq = (v % 2 == 1); 
			int g = v / 2; 

			if (onlyeq)
			{
				if (g == gen && vals[gen][get<1>(i)][get<0>(i)] % 4 != expected)
					return false;
			}
			else
			{
				if (g <= gen && vals[gen][get<1>(i)][get<0>(i)] % 4 != expected)
					return false;
			}
		}
		return true; 
	}

	int invalideCount(int gen, const vector<pair<int, int>>& v)
	{
		int t = 0; 

		for (auto i : v)
			if (vals[gen][i.second][i.first] % 4 == 0)
				t++;

		return t;
	}

	bool validategen(int gen)
	{
		return (invalideCount(gen, last_validator) <= last_violations) && validate(gen, static_validator);
	}

	bool validatetarget(int gen)
	{
		//return validate(gen, target_validator);
	}

	void zeroall()
	{
		for (int i = 0; i < MAXGEN; i++)
			for (int y = genbox[i][1]; y <= genbox[i][3]; y++)
				for (int x = genbox[i][0]; x <= genbox[i][2]; x++)
					vals[i][y][x] = 0; 

		for (int j = box[1]; j <= box[1] + box[3]; j++)
			for (int i = box[0]; i <= box[0] + box[2]; i++)
				for(int n = 0; n < 3; n++)
				{
					gen_start[n][j][i] = MAXGEN;
				}

		static_validator.clear();
		target_validator.clear();
		last_validator.clear();
	}

	void setup0(const vector<vector<int>>& a)
	{
		int rows = a.size();
		int cols = a[0].size();
		int dx = Size / 2 - rows / 2;
		int dy = Size / 2 - cols / 2;
		dx_init = dx; 
		dy_init = dy;
		//minx
		genbox[0][0] = dx - 1; 

		//miny
		genbox[0][1] = dy - 1;

		//maxx
		genbox[0][2] = dx + cols + 1;

		//maxy
		genbox[0][3] = dy + rows + 1;


		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				vals[0][dy + i][dx + j] = a[i][j];

				if(a[i][j] == 4)
					gen_start[0][dy + i][dx + j] = -1;

				if (a[i][j] % 4 == 1 && a[i][j] > 4)
					static_validator.push_back(make_tuple(dx + j, dy + i, a[i][j]));
			}
		}

		max_validation_gen = max_validation();
	}

	void print(int g) const
	{
		int minx = Size - 1;
		int maxx = 0;
		int miny = Size - 1;
		int maxy = 0;

		for (int y = box[1]; y <= box[1] + box[3]; y++)
			for (int x = box[0]; x <= box[0] + box[2]; x++)
				if (vals[g][y][x] % 4 != 0)
				{
					minx = min(x, minx);
					miny = min(y, miny);
					maxx = max(x, maxx);
					maxy = max(y, maxy);
				}

		//cout << "\n" << minx << ";" << miny << "\n";
		cout << "\n";

		for (int y = miny; y <= maxy; y++)
		{
			for (int x = minx; x <= maxx; x++)
			{
				if (vals[g][y][x] == 0)
					cout << ".";
				else if (vals[g][y][x] == 1)
					cout << "X";
				else if (vals[g][y][x] == 5)
					cout << "O";
				else if (vals[g][y][x] == 4)
					cout << ".";
				else
					cout << ".";
			}
			cout << "\n";

		}

		cout << "\n";
	}

	catinput  generate_input(int startgen, const vector<tuple<int, int, int>>& validations, const vector<tuple<int, int, int>>& prev) const
	{
		int minx = Size - 1;
		int maxx = 0;
		int miny = Size - 1;
		int maxy = 0;

		for (int y = 0; y < Size; y++)
			for (int x = 0; x < Size; x++)
				if (vals[0][y][x] % 4 != 0)
				{
					minx = min(x, minx);
					miny = min(y, miny);
					maxx = max(x, maxx);
					maxy = max(y, maxy);
				}

		vector<vector<int>> res = vector<vector<int>>(maxy - miny + 1, vector<int>(maxx - minx + 1, 0));

		for (int y = miny; y <= maxy; y++)
		{
			for (int x = minx; x <= maxx; x++)
			{
				res[y - miny][x - minx] = vals[0][y][x];
			}

		}

		catinput inp; 

		inp.init = res; 
		inp.firstgen = startgen; 

		for (auto i : validations)
			inp.validations.push_back(make_tuple(get<0>(i) - dx_init + minx, get<1>(i) - dy_init + miny, get<2>(i)));

		for (auto i : prev)
			inp.prevBlock.push_back(make_tuple(get<0>(i) - dx_init + minx, get<1>(i) - dy_init + miny, get<2>(i)));

		return inp; 
	}

	int count(int gen, int x, int y)
	{
		int total = 0; 

		for (int i = -1; i <= 1; i++)
			for (int j = -1; j <= 1; j++)
				total += vals[gen][y + j][x + i] % 4;

		return total; 

	}
	void evolvegen(int gen)
	{
		int minx = Size - 1;
		int maxx = 0;
		int miny = Size - 1;
		int maxy = 0;

		for (int y = genbox[gen - 1][1]; y <=  genbox[gen - 1][3]; y++)
			for (int x = genbox[gen - 1][0]; x <= genbox[gen - 1][2]; x++)
			{
				int t = count(gen - 1, x, y);
				int d = vals[gen - 1][y][x] % 4; 

				if (d == 0)
				{
					if (t == 0)
						vals[gen][y][x] = 4;
					else if (t == 1)
						vals[gen][y][x] = 8;
					else if (t == 2)
						vals[gen][y][x] = 12;
					else if (t == 3)
						vals[gen][y][x] = 1;
					else 
						vals[gen][y][x] = 16;
				}

				if (d == 1)
				{
					if (t == 1)
						vals[gen][y][x] = 20;
					else if (t == 2)
						vals[gen][y][x] = 20;
					else if (t == 3)
						vals[gen][y][x] = 5;
					else if (t == 4)
						vals[gen][y][x] = 5;
					else
						vals[gen][y][x] = 20;
				}

				if (vals[gen][y][x] % 4 == 1)
				{
					minx = min(x, minx);
					miny = min(y, miny);
					maxx = max(x, maxx);
					maxy = max(y, maxy);
				}
			}

		genbox[gen][0] = minx - 1;
		genbox[gen][1] = miny - 1;
		genbox[gen][2] = maxx + 1;
		genbox[gen][3] = maxy + 1;

	}

	void validate_trace(const vector<vector<int>>& trace, int dx, int dy, vector<pair<int, int>>& res)
	{
		for(int i = 0; i < trace.size(); i++)
			for (int j = 0; j < trace[0].size(); j++)
			{
				if (trace[i][j] == 0)
					continue;
				else if (trace[i][j] == 4 && gen_start[2][dy + i][dx + j] != MAXGEN)
					return;
				else if (trace[i][j] == 8 && gen_start[2][dy + i][dx + j] == MAXGEN)
					return; 
					
			}

		res.push_back(make_pair(dx, dy));

	}

	void fitbox()
	{
		int minx = Size, maxx = 0, miny = Size, maxy = 0;
		
		for (int i = 0; i < MAXGEN; i++)
		{
			minx = min(minx, genbox[i][0]);
			miny = min(miny, genbox[i][1]);
			maxx = max(maxx, genbox[i][2]);
			maxy = max(maxy, genbox[i][3]);
		}

		box = vector<int>{ minx, miny, maxx - minx, maxy - miny};

	}
	void evolveall()
	{
		max_gen = MAXGEN;

		for (int i = 1; i < MAXGEN; i++)
		{
			evolvegen(i);

			if (!validategen(i) && max_gen == MAXGEN)
				max_gen = i - 1;
		}

		max_gen += 5; 
		max_gen = min(max_gen, MAXGEN);

		fitbox();

		for (int y = box[1]; y <= box[1] + box[3]; y++)
			for (int x = box[0]; x <= box[0] + box[2]; x++)
			{
				for (int n = 0; n < 3; n++)
				{
					vector<int> a{ 8, 12, 1 };

					for (int i = 1; i < MAXGEN; i++)
					{
						if (vals[i][y][x] == a[n] || vals[i][y][x] % 4 == 1)
						{
							if(gen_start[n][y][x] >= 0)
								gen_start[n][y][x] = i;

							break;
						}
					}
				}
			}

		for (int n = 0; n < glider_traces.size(); n++)
		{
			vector<pair<int, int>> vals;

			for (int j = box[1]; j <= box[1] + box[3]; j++)
				for (int i = box[0]; i <= box[0] + box[2]; i++)
				{
					validate_trace(glider_traces[n], i, j, vals);
				}

			for (auto p : vals)
				for (int i = 0; i < glider_traces[n].size(); i++)
					for (int j = 0; j < glider_traces[n][0].size(); j++)
						if (glider_traces[n][i][j] != 0)
						{
							gen_start[2][p.second + i][p.first + j] = -1;
							gen_start[1][p.second + i][p.first + j] = -1;
							gen_start[0][p.second + i][p.first + j] = -1;
						}
		}
	}

	void AddCell(int gen, int x, int y, int val)
	{
		if (vals[gen][y][x] % 4 != val % 4)
		{
			vals[gen][y][x] = 16 * (vals[gen][y][x] + 4) + val;
			pertrubations[gen].push_back(make_pair(x, y));
		}
		else 
			vals[gen][y][x] = 16 * (vals[gen][y][x] + 4) + 8 + val;
	}

	void AddVec(const vector<pair<int, int>>& pert, const vector<pair<int, int>>& must, int gen, int dx, int dy)
	{
		for(int j = 0; j < pert.size(); j++)
		{
			AddCell(gen, dx + pert[j].first, dy + pert[j].second, 1);
		}
		
		last_validator.clear();
		for (int j = 0; j < must.size(); j++)
		{
			last_validator.push_back(make_pair(dx + must[j].first, dy + must[j].second));
		}

	}
	
	void Clean(int gen, int gen1, bool del)
	{
		for (auto pr : pertrubations[gen])
		{
			for (int i = -1; i <= 1; i++)
				for (int j = -1; j <= 1; j++)
				{
					int x = pr.first + i;
					int y = pr.second + j;

					if (vals[gen1][y][x] >= 64)
					{
						vals[gen1][y][x] /= 16;
						vals[gen1][y][x] -= 4;
					}
				}
		}

		if(del)
			pertrubations[gen].clear();
	}

	void CleanRange(int gens, int gene)
	{
		Clean(gens, gens, false);
		for (int i = gens; i <= gene && i + 1 < MAXGEN; i++)
			Clean(i, i + 1, true);
	}

	void Evolve(int gen)
	{
		for (auto pr : pertrubations[gen])
		{
			for (int i = -1; i <= 1; i++)
				for (int j = -1; j <= 1; j++)
				{
					int x = pr.first + i; 
					int y = pr.second + j;

					if (vals[gen + 1][y][x] < 64)
					{
						int t = count(gen, x, y);
						int d = vals[gen][y][x] % 4;
						int v = 0;

						if (d == 0)
						{
							if (t == 0)
								v = 4;
							else if (t == 1)
								v = 4;
							else if (t == 2)
								v = 4;
							else if (t == 3)
								v = 1;
							else
								v = 4; 
						}

						if (d == 1)
						{
							if (t == 1)
								v = 0;
							else if (t == 2)
								v = 0;
							else if (t == 3)
								v = 1;
							else if (t == 4)
								v = 1;
							else
								v = 0;
						}
						AddCell(gen + 1, x, y, v);
					}
				}
		}
	}

};


class catalyist
{
public:
	vector<vector<int>> cat;
	int max_violations; 

	vector<pair<int, int>> active;
	vector<pair<int, int>> must_live;

	vector<tuple<int, int, int>> zeros;


	int count(const vector<vector<int>>& v, int x, int y)
	{
		int total = 0;

		for (int i = -1; i <= 1; i++)
			for (int j = -1; j <= 1; j++)
				total += v[y + j][x + i] % 4;

		return total;

	}

	void init(int violations)
	{
		vec2zeros();
		vec2live();
		max_violations = violations;
	}

	void vec2live()
	{
		for (int j = 1; j < cat.size() - 1; j++)
			for (int i = 1; i < cat[0].size() - 1; i++)
			{
				if (cat[j][i] == 5)
					must_live.push_back(make_pair(i, j));

				if (cat[j][i] % 4 == 1)
					active.push_back(make_pair(i, j));
			}
			
	}

	void vec2zeros()
	{
		for (int j = 1; j < cat.size() - 1; j++)
			for (int i = 1; i < cat[0].size() - 1; i++)
			{
				if (cat[j][i] % 4 == 0)
				{
					int t = count(cat, i, j);

					int d = 0; 

					if (cat[j][i] == 4)
						d += 4;
					
					if (t == 1)
						zeros.push_back(make_tuple(i, j, d + 1));
					else if (t == 2)
						zeros.push_back(make_tuple(i, j, d));
				}

				
			}
	}

	int MinGen(int x, int y, const catalator& cater)
	{
		int ming = MAXGEN;
		bool valid = true; 

		for (auto xy : active)
		{
			int v = cater.vals[0][y + xy.second][x + xy.first];

			if (v != 0)
				return -1;
		}

		for (auto xyt : zeros)
		{
			int g = cater.gen_start[get<2>(xyt) % 4][y + get<1>(xyt)][x + get<0>(xyt)];

			if (g <= ming)
			{
				if (get<2>(xyt) >= 4)
					valid = false;
				else if (g < ming)
					valid = true; 

				ming = g;
			}
		}

		if (valid)
			return ming;
		else
			return -1; 
	}

	void MinGen(vector<vector<int>>& result, int& x, int& y, const catalator& cator)
	{
		x = cator.box[0] - cat[0].size() - 1;
		y = cator.box[1] - cat.size() - 1;

		result = vector<vector<int>>(2 * cat.size() + cator.box[3] + 1, vector<int>(2 * cat[0].size() + cator.box[2] + 1, MAXGEN));

		for (int j = 0; j < result.size(); j++)
			for (int i = 0; i < result[0].size(); i++)
			{
				int g = MinGen(x + i, y + j, cator);

				if (g <= cator.max_gen - 5 && g >= 0)
					result[j][i] = g - 1;
			}
	}

	bool validateZeros(int gen, int dx, int dy, const catalator& cator)
	{
		for (auto z : zeros)
			if (cator.vals[gen][get<1>(z) + dy][get<0>(z) + dx] % 4 != 0)
				return false; 

		return true; 
	}

	bool validateActive(int gen, int dx, int dy, const catalator& cator)
	{
		for (auto z : active)
			if (cator.vals[gen][z.second + dy][z.first + dx] % 4 != 1)
				return false;

		return true;
	}

	void setActive(int dx, int dy, int gen, int val, catalator& cator)
	{
		for (int j = 1; j < cat.size() - 1; j++)
			for (int i = 1; i < cat[0].size() - 1; i++)
				if (cat[j][i] != 0)
				{
					if (val == 1)
					{
						if(cat[j][i] % 4 == 0)
							cator.vals[0][dy + j][dx + i] = 4;
						else 
							cator.vals[0][dy + j][dx + i] = 4 * gen + 1;

					}
					else
						cator.vals[0][dy + j][dx + i] = 0;
				}
				
	}
};

void searchcats(vector<catinput>& result, const catinput& c,  vector<catalyist>& css, catalator& cat)
{
	cat.init(c.init, c.firstgen);
	cat.setup_validations(c.validations);

	cat.evolveall();

	for (int catidx = 0; catidx < css.size(); catidx++)
	{
		int x, y;
		vector<vector<int>> r;
		css[catidx].MinGen(r, x, y, cat);
		cat.last_violations = css[catidx].max_violations; 

		for (int j = 0; j < r.size(); j++)
			for (int i = 0; i < r[0].size(); i++)
			{
				int gen = r[j][i];

				if (gen < cat.max_gen && gen >= cat.firstactivegen)
				{
					cat.AddVec(css[catidx].active, css[catidx].must_live, gen, x + i, y + j);
					
					bool valid = false; 
					int k = 0; 

					int countvalid = 0; 

					for (; k < 15 && gen + k + 1 < MAXGEN; ++k)
					{
						cat.Evolve(gen + k);
						if (cat.validategen(gen + k + 1) == false)
						{
							valid = false; 
							break;
						}
						if (k > 2 && css[catidx].validateZeros(gen + k + 1, x + i, y + j, cat))
						{
							if (css[catidx].validateActive(gen + k + 1, x + i, y + j, cat))
							{
								countvalid++;
							}
						}
						else
							countvalid = 0; 
						
						if (countvalid >= 3)
						{
							valid = true; 
							break; 
						}
					}

					if (valid)
					{
						//result.push_back(make_tuple(x + i, y + j, catidx, gen));
						for (; gen + k < cat.max_validation_gen; k++)
						{
							cat.Evolve(gen + k);
							if (cat.validategen(gen + k + 1) == false)
							{
								valid = false;
								break;
							}
						}

						if (valid)
						{
							//TODO validate prev block:
							bool validate_prev = false; 

							for (; gen + k + 1 < MAXGEN; k++)
							{
								cat.Evolve(gen + k);
								if (cat.validategen(gen + k + 1) == false)
									break;
							}

							int u = cat.firstactivegen;
							for (; u < gen + k + 1; u++)
							{
								if (cat.validate(u, c.prevBlock))
								{
									validate_prev = true;
									break;
								}
							}

							auto inp = c; 

							if (validate_prev)
							{
								for (auto p : c.prevBlock)
								{

									if(get<2>(p) % 4 == 0)
										inp.validations.push_back(make_tuple(get<0>(p), get<1>(p), 4 * u));
									else 
										inp.validations.push_back(make_tuple(get<0>(p), get<1>(p), 4 * u + 1));

									inp.prevBlock = inp.curBlock;
									inp.curBlock.clear();
									inp.firstgen = u;
								}
							}

							css[catidx].setActive(x + i, y + j, gen + k, 1, cat);
							inp = cat.generate_input(gen, inp.validations, inp.prevBlock);
							result.push_back(inp);
							css[catidx].setActive(x + i, y + j, gen + k, 0, cat);

						}
					}

					cat.CleanRange(gen, gen + k);
				}
			}
	}

	cat.zeroall();
}

int main()
{
	//vector<vector<int>> c{ {5,0,0,0,0,0,0,0,0,4,4,4,4,4,4,4,0},{5,5,5,0,0,0,0,0,4,4,4,4,4,4,4,0,0},{0,0,0,5,0,0,4,4,4,4,4,4,4,4,0,0,0},{0,0,5,1,0,4,4,4,4,4,4,4,4,0,0,0,0},{0,0,0,0,0,4,4,4,4,4,4,4,0,0,0,0,0},{0,0,0,0,0,4,4,4,4,4,4,0,0,0,0,0,0},{0,0,0,0,0,1,4,4,4,4,0,0,0,0,0,0,0},{0,0,0,0,1,4,4,4,4,4,0,0,0,0,0,0,0},{0,0,0,0,1,1,1,4,4,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,1,5,0,0,0},{0,0,0,0,0,1,1,0,0,0,0,0,5,0,5,0,0},{0,0,0,0,0,1,1,0,0,0,0,0,0,0,5,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,5,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} };
	vector<vector<int>> c{ {5,0,0,0,0,0,0,0,0,4,4,4,4,4,4,4,4,0},{5,5,5,0,0,0,0,0,4,4,4,4,4,4,4,4,0,0},{0,0,0,5,0,0,0,4,4,4,4,4,4,4,4,0,0,0},{0,0,5,1,0,0,4,4,4,4,4,4,4,4,0,0,0,0},{0,0,0,0,0,4,4,4,4,4,4,4,4,0,0,0,0,0},{0,0,0,0,4,4,4,4,4,4,4,4,0,0,0,0,0,0},{0,0,0,0,0,1,4,4,4,4,4,0,0,0,0,0,0,0},{0,0,0,0,1,4,4,4,4,4,0,0,0,0,0,0,0,0},{0,0,0,0,1,1,1,0,4,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,1,5,0,0,0,0},{0,0,0,0,0,1,1,0,0,0,0,0,5,0,5,0,0,0},{0,0,0,0,0,1,1,0,0,0,0,0,0,0,5,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,5,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} };

	vector<vector<int>> a0 {{0,0,0,0,0,0,0,0,0},{0,8,8,8,8,0,0,0,0},{0,4,1,5,4,0,0,0,0},{0,4,5,4,4,4,4,0,0},{0,4,4,5,5,5,4,0,0},{0,0,4,4,4,5,8,0,0},{0,0,0,0,4,4,4,0,0},{0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0}};
	vector<vector<int>> a1 {{0,0,0,0,0,0,0,0,0},{0,0,0,4,4,4,8,0,0},{0,0,4,4,5,1,8,0,0},{0,0,4,5,4,5,8,0,0},{0,4,4,5,4,4,8,0,0},{0,4,5,5,4,0,0,0,0},{0,4,8,4,4,0,0,0,0},{0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0}};

	vector<catalyist> css;
	
	for (int i = 0; i < 4; i++)
	{
		catalyist cs0;
		cs0.cat = flip(a0, i);
		cs0.init(0);

		css.push_back(cs0);

		catalyist cs1;
		cs1.cat = flip(a1, i);
		cs1.init(0);

		css.push_back(cs1);
	}
	
	vector<vector<int>> b{ {0,0,0,0,0,0,0},{0,8,8,8,8,0,0},{0,8,5,5,8,0,0},{0,8,5,5,8,0,0},{0,8,8,8,8,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0} }; 
	catalyist cs;
	cs.cat = b;
	cs.init(2);
	css.push_back(cs);

	vector<catinput> result;
	catalator cat;

	catinput firstC; 
	firstC.firstgen = 15; 
	firstC.init = c;
	firstC.validations = vector<tuple<int, int, int>> { make_tuple(-2, 7, 98), make_tuple(-1, 7, 98), make_tuple(0, 7, 98), make_tuple(1, 7, 98), make_tuple(2, 7, 98), make_tuple(-2, 8, 98), make_tuple(-1, 8, 99), make_tuple(0, 8, 99), make_tuple(1, 8, 99), make_tuple(2, 8, 98), make_tuple(-2, 9, 98), make_tuple(-1, 9, 99), make_tuple(0, 9, 98), make_tuple(1, 9, 98), make_tuple(2, 9, 98), make_tuple(-2, 10, 98), make_tuple(-1, 10, 98), make_tuple(0, 10, 99), make_tuple(1, 10, 98), make_tuple(-1, 11, 98), make_tuple(0, 11, 98), make_tuple(1, 11, 98), make_tuple(2, 12, 99) };
	firstC.prevBlock = vector<tuple<int, int, int>> { make_tuple(4, 9, 8), make_tuple(5, 9, 8), make_tuple(6, 9, 8), make_tuple(7, 9, 8), make_tuple(3, 10, 8), make_tuple(4, 10, 8), make_tuple(5, 10, 8), make_tuple(6, 10, 8), make_tuple(7, 10, 8), make_tuple(8, 10, 8), make_tuple(3, 11, 8), make_tuple(4, 11, 8), make_tuple(5, 11, 9), make_tuple(6, 11, 9), make_tuple(7, 11, 8), make_tuple(8, 11, 8), make_tuple(3, 12, 8), make_tuple(4, 12, 8), make_tuple(5, 12, 9), make_tuple(6, 12, 9), make_tuple(7, 12, 8), make_tuple(8, 12, 8), make_tuple(3, 13, 8), make_tuple(4, 13, 8), make_tuple(5, 13, 8), make_tuple(6, 13, 8), make_tuple(7, 13, 8), make_tuple(8, 13, 8), make_tuple(4, 14, 8), make_tuple(5, 14, 8), make_tuple(6, 14, 8), make_tuple(7, 14, 8) };
	
	searchcats(result, firstC, css, cat);
	
	for (int i = 0; i < result.size(); i++)
	{
		print(result[i].init);
		cout << ".\n.\n.\n.\n.\n.\n.\n.\n.\n.\n.\n.\n.\n.\n.\n.\n.\n.\n.\n.\n.\n.\n.\n";
	}
}