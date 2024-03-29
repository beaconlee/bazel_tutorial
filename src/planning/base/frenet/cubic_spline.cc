#include "cubic_spline.h"

using std::vector;
using namespace Eigen;


template <typename T>
std::vector<T> diff(const std::vector<T>& vec)
{
  std::vector<T> ret;
  for(size_t idx = 1; idx < vec.size(); ++idx)
  {
    ret.push_back(vec[idx] - vec[idx - 1]);
  }

  return ret;
}

template <typename T>
std::vector<T> cumsum(std::vector<T> vec)
{
  std::vector<T> output;
  T tmp = 0;
  for(size_t idx = 0; idx < vec.size(); ++idx)
  {
    tmp += vec[idx];
    output.push_back(tmp);
  }

  return output;
}


CubicSpline::CubicSpline(vector<double> _x, vector<double> _y)
{
  x = _x;
  y = _y;
  nx = x.size();
  h = diff(x);
  // 判断所有的h是否都有效
  bool not_valid =
      std::any_of(h.begin(), h.end(), [](double val) { return val < 0; });

  if(not_valid)
  {
    throw std::invalid_argument(
        "x coordinates must be sorted in ascending order");
  }
  a = y;
  MatrixXd A = calc_A();
  VectorXd B = calc_B();
  VectorXd c_eigen = A.colPivHouseholderQr().solve(B);
  double* c_pointer = c_eigen.data();
  // 在 C++ 中，std::vector 的 assign 函数用于将新元素赋值给向量，替换其当前内容。assign 函数有多个重载形式，可以接受不同类型的参数，以及不同的范围来指定要分配给向量的值。
  c.assign(c_pointer, c_pointer + c_eigen.rows());

  for(size_t idx = 0; idx < (nx - 1); ++idx)
  {
    double d_tmp = (c[idx + 1] - c[idx]) / (3.0 * h[idx]);
    double b_tmp = (a[idx + 1] - a[idx]) / h[idx] -
                   h[idx] * (c[idx + 1] + 2 * c[idx]) / 3.0;
    d.emplace_back(d_tmp);
    b.emplace_back(b_tmp);
  }
}

MatrixXd CubicSpline::calc_A(void)
{
  // 根据 x 的大小初始化一个方阵
  // 矩阵 A：矩阵 A 是一个 nx × nx 的对称三对角矩阵，其中 nx 是插值节点的数量。矩阵 A 的主对角线和两条相邻的对角线上的元素用于表示插值函数的二阶导数，而其他元素都为零。具体地，对于内部节点 i，矩阵 A 的对角线元素为 2*(h[i-1]+h[i])，其中 h[i] 表示节点 i 的步长（即相邻节点间的距离）。矩阵 A 的相邻对角线元素为 h[i-1]，表示相邻节点之间的关系。首尾节点处，由于缺少相邻节点，对应的元素为特殊值。
  MatrixXd A = MatrixXd::Zero(nx, nx);
  A(0, 0) = 1.0;


  /*
  1  1  1  1  1  1  1  1  1  1  1  1
  1  1  1  1  1  1  1  1  1  1  1  1
  1  1  1  1  1  1  1  1  1  1  1  1
  1  1  1  1  1  1  1  1  1  1  1  1
  1  1  1  1  1  1  1  1  1  1  1  1
  1  1  1  1  1  1  1  1  1  1  1  1
  1  1  1  1  1  1  1  1  1  1  1  1
  1  1  1  1  1  1  1  1  1  1  1  1
  1  1  1  1  1  1  1  1  1  1  1  1
  1  1  1  1  1  1  1  1  1  1  1  1
  1  1  1  1  1  1  1  1  1  1  1  1
  */
  for(size_t idx = 0; idx < (nx - 1); ++idx)
  {
    if(idx != nx - 2)
    {
      // 这里赋值的是对角线上的点
      // 从 第二行 赋值到 倒数第二行
      // 因为根据四个条件只能确定 4n-2 个方程
      // 还有两个方程有边界条件确定
      A(idx + 1, idx + 1) = 2.0 * (h[idx] + h[idx + 1]);
    }
    // 这里赋值的是 上面对角点 左边的点
    A(idx + 1, idx) = h[idx];
    // 这里赋值的是 上面对角点 左边的点
    A(idx, idx + 1) = h[idx];
  }
  // 这里确定最后两个方程，使用一些边界条件
  // 使用  自由边界(Natural)
  A(0, 1) = 0.0;
  A(nx - 1, nx - 2) = 0.0;
  A(nx - 1, nx - 1) = 1.0;

  return A;
}

VectorXd CubicSpline::calc_B(void)
{
  VectorXd B = VectorXd::Zero(nx);

  for(size_t idx = 0; idx < (nx - 2); ++idx)
  {
    B(idx + 1) = 3.0 * (a[idx + 2] - a[idx + 1]) / h[idx + 1] -
                 3.0 * (a[idx + 1] - a[idx]) / h[idx];
  }

  return B;
}

double CubicSpline::calc_position(double _x) const
{
  if(_x < x[0] || _x > x.back())
  {
    throw std::invalid_argument("received value out of the pre-defined range");
  }

  auto it = std::upper_bound(x.begin(), x.end(), _x);
  int idx = std::distance(x.begin(), it) - 1;
  double dx = _x - x[idx];
  double position =
      a[idx] + b[idx] * dx + c[idx] * pow(dx, 2) + d[idx] * pow(dx, 3);

  return position;
}

double CubicSpline::calc_first_derivative(double _x) const
{
  if(_x < x[0] || _x > x.back())
  {
    throw std::invalid_argument("received value out of the pre-defined range");
  }

  auto it = std::upper_bound(x.begin(), x.end(), _x);
  int idx = std::distance(x.begin(), it) - 1;
  double dx = _x - x[idx];
  double dy = b[idx] + 2.0 * c[idx] * dx + 3.0 * d[idx] * pow(dx, 2);

  return dy;
}

double CubicSpline::calc_second_derivative(double _x) const
{
  if(_x < x[0] || _x > x.back())
  {
    throw std::invalid_argument("received value out of the pre-defined range");
  }

  auto it = std::upper_bound(x.begin(), x.end(), _x);
  int idx = std::distance(x.begin(), it) - 1;
  double dx = _x - x[idx];
  double ddy = 2.0 * c[idx] + 6.0 * d[idx] * dx;

  return ddy;
}

double CubicSpline::operator()(double _x, int dd) const
{
  double value = 0.0;
  if(dd < 0 || dd > 2)
  {
    throw std::invalid_argument("received value out of the pre-defined range");
  }

  if(dd == 0)
  {
    value = calc_position(_x);
  }
  else if(dd == 1)
  {
    value = calc_first_derivative(_x);
  }
  else if(dd == 2)
  {
    value = calc_second_derivative(_x);
  }

  return value;
}

CubicSpline2D::CubicSpline2D(vector<double> _x, vector<double> _y)
{
  // s 就相当于 sl 坐标系中的 s
  // 表示 s 方向的距离
  s = calc_s(_x, _y);
  //
  sx = CubicSpline(s, _x);
  sy = CubicSpline(s, _y);
} 

vector<double> CubicSpline2D::calc_s(vector<double> _x, vector<double> _y)
{
  vector<double> dx = diff(_x);
  vector<double> dy = diff(_y);
  vector<double> ds;
  vector<double> s = {0};
  // 计算两点之间的欧几里得距离 欧氏距离（Euclidean Distance）
  for(size_t idx = 0; idx < dx.size(); ++idx)
  {
    ds.push_back(hypot(dx[idx], dy[idx]));
  }
  // ds 中之前存放的是两点之间间隔的距离
  // 现在将 ds 中的距离进行累加
  vector<double> cum = cumsum(ds);
  s.insert(s.end(), cum.begin(), cum.end());

  return s;
}

// 这样设计的好处是：可以简化计算
// 只需要输入预估目标距离当前点的距离就可以计算出坐标
// 如果还采用笛卡尔坐标系的话，需要根据距离计算出预估点的
Vector2d CubicSpline2D::calc_position(double _s) const
{
  double _x = sx.calc_position(_s);
  double _y = sy.calc_position(_s);

  return {_x, _y};
}

double CubicSpline2D::calc_yaw(double _s) const
{
  double dx = sx.calc_first_derivative(_s);
  double dy = sy.calc_first_derivative(_s);
  double yaw = atan2(dy, dx);

  return yaw;
}

double CubicSpline2D::calc_curvature(double _s) const
{
  double dx = sx.calc_first_derivative(_s);
  double ddx = sx.calc_second_derivative(_s);
  double dy = sy.calc_first_derivative(_s);
  double ddy = sy.calc_second_derivative(_s);
  double k = (ddy * dx - ddx * dy) / pow(dx * dx + dy * dy, 1.50);

  return k;
}

Vector2d CubicSpline2D::operator()(double _s, int n) const
{
  Vector2d p(sx(_s, n), sy(_s, n));

  return p;
}

vector<vector<double>>
CubicSpline2D::calc_spline_course(vector<double> x, vector<double> y, double ds)
{
  CubicSpline2D* sp = new CubicSpline2D(x, y);
  vector<vector<double>> output(4);

  for(double s = sp->s.front(); s < sp->s.back(); s += ds)
  {
    Vector2d ixy = sp->calc_position(s);
    output[0].push_back(ixy[0]);
    output[1].push_back(ixy[1]);
    output[2].push_back(sp->calc_yaw(s));
    output[3].push_back(sp->calc_curvature(s));
  }
  delete sp;

  // [x, y, yaw, curvature]
  return output;
}
