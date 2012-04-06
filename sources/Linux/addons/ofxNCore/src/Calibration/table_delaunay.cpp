#include "table_delaunay.h"

char ch_cap ( char c )
{
	if ( 97 <= c && c <= 122 ) { c = c - 32; }
	return c;
}


bool ch_eqi ( char c1, char c2 )
{
    if ( 97 <= c1 && c1 <= 122 ) { c1 = c1 - 32; }
    if ( 97 <= c2 && c2 <= 122 ) { c2 = c2 - 32; }
    return ( c1 == c2 );
}


int ch_to_digit ( char c )
{
    int digit;
    if ( '0' <= c && c <= '9' ) { digit = c - '0'; }
    else if ( c == ' ' ) { digit = 0; }
    else { digit = -1; }
    return digit;
}


int diaedg ( double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3 )
{
    double ca;
    double cb;
    double dx10;
    double dx12;
    double dx30;
    double dx32;
    double dy10;
    double dy12;
    double dy30;
    double dy32;
    double s;
    double tol;
    double tola;
    double tolb;
    int value;

    tol = 100.0 * r8_epsilon ( );

    dx10 = x1 - x0;
    dy10 = y1 - y0;
    dx12 = x1 - x2;
    dy12 = y1 - y2;
    dx30 = x3 - x0;
    dy30 = y3 - y0;
    dx32 = x3 - x2;
    dy32 = y3 - y2;

    tola = tol * r8_max ( fabs ( dx10 ), r8_max ( fabs ( dy10 ), r8_max ( fabs ( dx30 ), fabs ( dy30 ) ) ) );
    tolb = tol * r8_max ( fabs ( dx12 ), r8_max ( fabs ( dy12 ), r8_max ( fabs ( dx32 ), fabs ( dy32 ) ) ) );

    ca = dx10 * dx30 + dy10 * dy30;
    cb = dx12 * dx32 + dy12 * dy32;

    if ( tola < ca && tolb < cb )
    {
        value = -1;
    }
    else if ( ca < -tola && cb < -tolb )
    {
        value = 1;
    }
    else
    {
        tola = r8_max ( tola, tolb );
        s = ( dx10 * dy30 - dx30 * dy10 ) * cb + ( dx32 * dy12 - dx12 * dy32 ) * ca;
        if ( tola < s )
        {
            value = -1;
        }
        else if ( s < -tola )
        {
            value = 1;
        }
        else
        {
            value = 0;
        }
    }
    return value;
}


int dtris2 ( int point_num, int base, double point_xy[], int *tri_num, int tri_vert[], int tri_nabe[] )
{
    double cmax, tol;
    int e, error, i, j, k, l, ledg, lr, ltri, m, m1, m2, n, redg, rtri, t, top;
    int *indx, *stack;

    stack = new int[point_num];
    tol = 100.0 * r8_epsilon ( );
    //
    //  Sort the vertices by increasing (x,y).
    //
    indx = r82vec_sort_heap_index_a ( point_num, base, point_xy );
    r82vec_permute ( point_num, indx, base, point_xy );
    //
    //  Make sure that the data points are "reasonably" distinct.
    //
    m1 = 1;

    for ( i = 2; i <= point_num; i++ )
    {
        m = m1;
        m1 = i;

        k = -1;

        for ( j = 0; j <= 1; j++ )
        {
            cmax = r8_max ( fabs ( point_xy[2*(m-1)+j] ),
            fabs ( point_xy[2*(m1-1)+j] ) );

            if ( tol * ( cmax + 1.0 ) < fabs ( point_xy[2*(m-1)+j] - point_xy[2*(m1-1)+j] ) )
            {
                k = j;
                break;
            }
        }

        if ( k == -1 )
        {
            std::cout << "\n";
            std::cout << "DTRIS2 - Fatal error!\n";
            std::cout << "  Fails for point number I = " << i << "\n";
            std::cout << "  M =  " << m  << "\n";
            std::cout << "  M1 = " << m1 << "\n";
            std::cout << "  X,Y(M)  = " << point_xy[2*(m-1)+0] << "  " << point_xy[2*(m-1)+1] << "\n";
            std::cout << "  X,Y(M1) = " << point_xy[2*(m1-1)+0] << "  " << point_xy[2*(m1-1)+1] << "\n";
            delete [] stack;
            return 224;
        }

    }
    //
    //  Starting from points M1 and M2, search for a third point M that
    //  makes a "healthy" triangle (M1,M2,M)
    //
    m1 = 1;
    m2 = 2;
    j = 3;

    for ( ; ; )
    {
        if ( point_num < j )
        {
            std::cout << "\n";
            std::cout << "DTRIS2 - Fatal error!\n";
            delete [] stack;
            return 225;
        }
        m = j;
        lr = lrline ( point_xy[2*(m-1)+0], point_xy[2*(m-1)+1],
        point_xy[2*(m1-1)+0], point_xy[2*(m1-1)+1],
        point_xy[2*(m2-1)+0], point_xy[2*(m2-1)+1], 0.0 );
        if ( lr != 0 )
        {
            break;
        }
        j = j + 1;
    }
    //
    //  Set up the triangle information for (M1,M2,M), and for any other
    //  triangles you created because points were collinear with M1, M2.
    //
    *tri_num = j - 2;

    if ( lr == -1 )
    {
        tri_vert[3*0+0] = m1;
        tri_vert[3*0+1] = m2;
        tri_vert[3*0+2] = m;
        tri_nabe[3*0+2] = -3;

        for ( i = 2; i <= *tri_num; i++ )
        {
            m1 = m2;
            m2 = i+1;
            tri_vert[3*(i-1)+0] = m1;
            tri_vert[3*(i-1)+1] = m2;
            tri_vert[3*(i-1)+2] = m;
            tri_nabe[3*(i-1)+0] = -3 * i;
            tri_nabe[3*(i-1)+1] = i;
            tri_nabe[3*(i-1)+2] = i - 1;
        }
        tri_nabe[3*(*tri_num-1)+0] = -3 * (*tri_num) - 1;
        tri_nabe[3*(*tri_num-1)+1] = -5;
        ledg = 2;
        ltri = *tri_num;
    }
    else
    {
        tri_vert[3*0+0] = m2;
        tri_vert[3*0+1] = m1;
        tri_vert[3*0+2] = m;
        tri_nabe[3*0+0] = -4;

        for ( i = 2; i <= *tri_num; i++ )
        {
            m1 = m2;
            m2 = i+1;
            tri_vert[3*(i-1)+0] = m2;
            tri_vert[3*(i-1)+1] = m1;
            tri_vert[3*(i-1)+2] = m;
            tri_nabe[3*(i-2)+2] = i;
            tri_nabe[3*(i-1)+0] = -3 * i - 3;
            tri_nabe[3*(i-1)+1] = i - 1;
        }

        tri_nabe[3*(*tri_num-1)+2] = -3 * (*tri_num);
        tri_nabe[3*0+1] = -3 * (*tri_num) - 2;
        ledg = 2;
        ltri = 1;
    }
    //
    //  Insert the vertices one at a time from outside the convex hull,
    //  determine visible boundary edges, and apply diagonal edge swaps until
    //  Delaunay triangulation of vertices (so far) is obtained.
    //
    top = 0;

    for ( i = j+1; i <= point_num; i++ )
    {
        m = i;
        m1 = tri_vert[3*(ltri-1)+ledg-1];

        if ( ledg <= 2 )
        {
            m2 = tri_vert[3*(ltri-1)+ledg];
        }
        else
        {
            m2 = tri_vert[3*(ltri-1)+0];
        }

        lr = lrline ( point_xy[2*(m-1)+0], point_xy[2*(m-1)+1],
        point_xy[2*(m1-1)+0], point_xy[2*(m1-1)+1],
        point_xy[2*(m2-1)+0], point_xy[2*(m2-1)+1], 0.0 );

        if ( 0 < lr )
        {
            rtri = ltri;
            redg = ledg;
            ltri = 0;
        }
        else
        {
            l = -tri_nabe[3*(ltri-1)+ledg-1];
            rtri = l / 3;
            redg = (l % 3) + 1;
        }

        vbedg ( point_xy[2*(m-1)+0], point_xy[2*(m-1)+1], point_num,
        point_xy, *tri_num, tri_vert, tri_nabe, &ltri, &ledg, &rtri, &redg );

        n = *tri_num + 1;
        l = -tri_nabe[3*(ltri-1)+ledg-1];

        for ( ; ; )
        {
            t = l / 3;
            e = ( l % 3 ) + 1;
            l = -tri_nabe[3*(t-1)+e-1];
            m2 = tri_vert[3*(t-1)+e-1];

            if ( e <= 2 )
            {
                m1 = tri_vert[3*(t-1)+e];
            }
            else
            {
                m1 = tri_vert[3*(t-1)+0];
            }

            *tri_num = *tri_num + 1;
            tri_nabe[3*(t-1)+e-1] = *tri_num;
            tri_vert[3*(*tri_num-1)+0] = m1;
            tri_vert[3*(*tri_num-1)+1] = m2;
            tri_vert[3*(*tri_num-1)+2] = m;
            tri_nabe[3*(*tri_num-1)+0] = t;
            tri_nabe[3*(*tri_num-1)+1] = *tri_num - 1;
            tri_nabe[3*(*tri_num-1)+2] = *tri_num + 1;
            top = top + 1;

            if ( point_num < top )
            {
                std::cout << "\n";
                std::cout << "DTRIS2 - Fatal error!\n";
                std::cout << "  Stack overflow.\n";
                delete [] stack;
                return 8;
            }

            stack[top-1] = *tri_num;

            if ( t == rtri && e == redg )
            {
                break;
            }

        }

        tri_nabe[3*(ltri-1)+ledg-1] = -3 * n - 1;
        tri_nabe[3*(n-1)+1] = -3 * (*tri_num) - 2;
        tri_nabe[3*(*tri_num-1)+2] = -l;
        ltri = n;
        ledg = 2;

        error = swapec ( m, &top, &ltri, &ledg, point_num, point_xy, *tri_num,
        tri_vert, tri_nabe, stack );

        if ( error != 0 )
        {
            std::cout << "\n";
            std::cout << "DTRIS2 - Fatal error!\n";
            std::cout << "  Error return from SWAPEC.\n";
            delete [] stack;
            return error;
        }
    }
    //
    //  Now account for the sorting that we did.
    //
    for ( i = 0; i < 3; i++ )
    {
        for ( j = 0; j < *tri_num; j++ )
        {
            tri_vert[i+j*3] = indx [ tri_vert[i+j*3] - 1 ];
        }
    }

    perm_inverse ( point_num, indx );

    r82vec_permute ( point_num, indx, base, point_xy );

    delete [] indx;
    delete [] stack;

    return 0;
}

int i4_max ( int i1, int i2 )
{
    int value;
    if ( i2 < i1 ) { value = i1; }
    else { value = i2; }
    return value;
}


int i4_min ( int i1, int i2 )
{
    int value;
    if ( i1 < i2 ) { value = i1; }
    else { value = i2; }
    return value;
}


int i4_modp ( int i, int j )
{
    int value;
    if ( j == 0 )
    {
        std::cout << "\n";
        std::cout << "I4_MODP - Fatal error!\n";
        std::cout << "  I4_MODP ( I, J ) called with J = " << j << "\n";
        exit ( 1 );
    }
    value = i % j;
    if ( value < 0 )
    {
        value = value + abs ( j );
    }
    return value;
}


int i4_sign ( int i )
{
    int value;
    if ( i < 0 )
    {
        value = -1;
    }
    else
    {
        value = 1;
    }
    return value;
}


int i4_wrap ( int ival, int ilo, int ihi )
{
    int jhi;
    int jlo;
    int value;
    int wide;

    jlo = i4_min ( ilo, ihi );
    jhi = i4_max ( ilo, ihi );

    wide = jhi + 1 - jlo;

    if ( wide == 1 )
    {
        value = jlo;
    }
    else
    {
        value = jlo + i4_modp ( ival - jlo, wide );
    }
    return value;
}


void i4mat_transpose_print_some ( int m, int n, int a[], int ilo, int jlo, int ihi, int jhi, std::string title )
{
    # define INCX 10

    int i;
    int i2hi;
    int i2lo;
    int j;
    int j2hi;
    int j2lo;

    std::cout << "\n";
    std::cout << title << "\n";
    //
    //  Print the columns of the matrix, in strips of INCX.
    //
    for ( i2lo = ilo; i2lo <= ihi; i2lo = i2lo + INCX )
    {
        i2hi = i2lo + INCX - 1;
        i2hi = i4_min ( i2hi, m );
        i2hi = i4_min ( i2hi, ihi );

        std::cout << "\n";
        //
        //  For each row I in the current range...
        //
        //  Write the header.
        //
        std::cout << "  Row: ";
        for ( i = i2lo; i <= i2hi; i++ )
        {
            std::cout << std::setw(6) << i << "  ";
        }
        std::cout << "\n";
        std::cout << "  Col\n";
        std::cout << "\n";
        //
        //  Determine the range of the rows in this strip.
        //
        j2lo = i4_max ( jlo, 1 );
        j2hi = i4_min ( jhi, n );

        for ( j = j2lo; j <= j2hi; j++ )
        {
            //
            //  Print out (up to INCX) entries in column J, that lie in the current strip.
            //
            std::cout << std::setw(5) << j << "  ";
            for ( i = i2lo; i <= i2hi; i++ )
            {
                std::cout << std::setw(6) << a[i-1+(j-1)*m] << "  ";
            }
            std::cout << "\n";
        }
    }

    return;
    # undef INCX
}


void i4mat_write (int m, int n, int table[] )
{
    int i;
    int j;
    //
    //  Write the data.
    //
    for ( j = 0; j < n; j++ )
    {
        for ( i = 0; i < m; i++ )
        {
            std::cout << "  " << std::setw(10) << table[i+j*m];
        }
        std::cout << "\n";
    }

    return;
}

void i4mat_tomat (int m, int n, int table[], std::vector<calibrationPoint> *cp, std::vector<triangle*>* t)
{
    int j;
    for ( j = 0; j < n; j++ )
    {
        t->push_back(new triangle(&cp->at(table[j*m]-1),&cp->at(table[1+j*m]-1),&cp->at(table[2+j*m]-1)));
    }
    return;
}


int *i4vec_indicator_new ( int n )
{
    int *a;
    int i;

    a = new int[n];

    for ( i = 0; i < n; i++ )
    {
        a[i] = i + 1;
    }
    return a;
}


int i4vec_min ( int n, int a[] )
{
    int i;
    int value;

    if ( n <= 0 )
    {
        return 0;
    }
    value = a[0];
    for ( i = 1; i < n; i++ )
    {
        if ( a[i] < value )
        {
            value = a[i];
        }
    }
    return value;
}


int lrline ( double xu, double yu, double xv1, double yv1, double xv2, double yv2, double dv )
{
    double dx;
    double dxu;
    double dy;
    double dyu;
    double t;
    double tol;
    double tolabs;
    int value;

    tol = 100.0 * r8_epsilon ( );

    dx = xv2 - xv1;
    dy = yv2 - yv1;
    dxu = xu - xv1;
    dyu = yu - yv1;

    tolabs = tol * r8_max ( fabs ( dx ), r8_max ( fabs ( dy ), r8_max ( fabs ( dxu ), r8_max ( fabs ( dyu ), fabs ( dv ) ) ) ) );

    t = dy * dxu - dx * dyu + dv * sqrt ( dx * dx + dy * dy );

    if ( tolabs < t )
    {
        value = 1;
    }
    else if ( -tolabs <= t )
    {
        value = 0;
    }
    else if ( t < -tolabs )
    {
        value = -1;
    }

    return value;
}


bool perm_check ( int n, int p[], int base )
{
    bool found;
    int i;
    int seek;

    for ( seek = base; seek < base + n; seek++ )
    {
        found = false;
        for ( i = 0; i < n; i++ )
        {
            if ( p[i] == seek )
            {
                found = true;
                break;
            }
        }
        if ( !found )
        {
            return false;
        }
    }
    return true;
}


void perm_inverse ( int n, int p[] )
{
  int base;
  int i;
  int i0;
  int i1;
  int i2;
  int is;
  int p_min;

  if ( n <= 0 )
  {
    std::cout << "\n";
    std::cout << "PERM_INVERSE - Fatal error!\n";
    std::cout << "  Input value of N = " << n << "\n";
    exit ( 1 );
  }
//
//  Find the least value, and shift data so it begins at 1.
//
  p_min = i4vec_min ( n, p );
  base = 1;

  for ( i = 0; i < n; i++ )
  {
    p[i] = p[i] - p_min + base;
  }
//
//  Now we can safely check the permutation.
//
  if ( !perm_check ( n, p, base ) )
  {
    std::cerr << "\n";
    std::cerr << "PERM_INVERSE - Fatal error!\n";
    std::cerr << "  PERM_CHECK rejects this permutation.\n";
    exit ( 1 );
  }
//
//  Now we can invert the permutation.
//
  is = 1;

  for ( i = 1; i <= n; i++ )
  {
    i1 = p[i-1];

    while ( i < i1 )
    {
      i2 = p[i1-1];
      p[i1-1] = -i2;
      i1 = i2;
    }

    is = - i4_sign ( p[i-1] );
    p[i-1] = i4_sign ( is ) * abs ( p[i-1] );
  }

  for ( i = 1; i <= n; i++ )
  {
    i1 = - p[i-1];

    if ( 0 <= i1 )
    {
      i0 = i;

      for ( ; ; )
      {
        i2 = p[i1-1];
        p[i1-1] = i0;

        if ( i2 < 0 )
        {
          break;
        }
        i0 = i1;
        i1 = i2;
      }
    }
  }
//
//  Now we can restore the permutation.
//
  for ( i = 0; i < n; i++ )
  {
    p[i] = p[i] + p_min - base;
  }

  return;
}


double r8_abs ( double x )
{
  double value;

  if ( 0.0 <= x )
  {
    value = x;
  }
  else
  {
    value = - x;
  }
  return value;
}


double r8_epsilon ( )
{
  double value;

  value = 1.0;

  while ( 1.0 < ( double ) ( 1.0 + value )  )
  {
    value = value / 2.0;
  }

  value = 2.0 * value;

  return value;
}


double r8_huge ( )
{
  double value;

  value = 1.0E+30;

  return value;
}


double r8_max ( double x, double y )
{
  double value;

  if ( y < x )
  {
    value = x;
  }
  else
  {
    value = y;
  }
  return value;
}


double r8_min ( double x, double y )
{
  double value;

  if ( y < x )
  {
    value = y;
  }
  else
  {
    value = x;
  }
  return value;
}


int r8_nint ( double x )
{
  int value;

  if ( x < 0.0 )
  {
    value = - ( int ) ( r8_abs ( x ) + 0.5 );
  }
  else
  {
    value =   ( int ) ( r8_abs ( x ) + 0.5 );
  }

  return value;
}


void r82vec_permute ( int n, int p[], int base, double a[] )
{
  double a_temp[2];
  int i;
  int iget;
  int iput;
  int istart;

  if ( !perm_check ( n, p, base ) )
  {
    std::cerr << "\n";
    std::cerr << "R82VEC_PERMUTE - Fatal error!\n";
    std::cerr << "  PERM_CHECK rejects this permutation.\n";
    exit ( 1 );
  }
//
//  In order for the sign negation trick to work, we need to assume that the
//  entries of P are strictly positive.  Presumably, the lowest number is BASE.
//  So temporarily add 1-BASE to each entry to force positivity.
//
  for ( i = 0; i < n; i++ )
  {
    p[i] = p[i] + 1 - base;
  }
//
//  Search for the next element of the permutation that has not been used.
//
  for ( istart = 1; istart <= n; istart++ )
  {
    if ( p[istart-1] < 0 )
    {
      continue;
    }
    else if ( p[istart-1] == istart )
    {
      p[istart-1] = - p[istart-1];
      continue;
    }
    else
    {
      a_temp[0] = a[0+(istart-1)*2];
      a_temp[1] = a[1+(istart-1)*2];
      iget = istart;
//
//  Copy the new value into the vacated entry.
//
      for ( ; ; )
      {
        iput = iget;
        iget = p[iget-1];

        p[iput-1] = - p[iput-1];

        if ( iget < 1 || n < iget )
        {
          std::cout << "\n";
          std::cout << "R82VEC_PERMUTE - Fatal error!\n";
          std::cout << "  Entry IPUT = " << iput << " of the permutation has\n";
          std::cout << "  an illegal value IGET = " << iget << ".\n";
          exit ( 1 );
        }

        if ( iget == istart )
        {
          a[0+(iput-1)*2] = a_temp[0];
          a[1+(iput-1)*2] = a_temp[1];
          break;
        }
        a[0+(iput-1)*2] = a[0+(iget-1)*2];
        a[1+(iput-1)*2] = a[1+(iget-1)*2];
      }
    }
  }
//
//  Restore the signs of the entries.
//
  for ( i = 0; i < n; i++ )
  {
    p[i] = - p[i];
  }
//
//  Restore the base of the entries.
//
  for ( i = 0; i < n; i++ )
  {
    p[i] = p[i] - 1 + base;
  }
  return;
}


int *r82vec_sort_heap_index_a ( int n, int base, double a[] )
{
  double aval[2];
  int i;
  int *indx;
  int indxt;
  int ir;
  int j;
  int l;

  if ( n < 1 )
  {
    return NULL;
  }

  indx = new int[n];

  for ( i = 0; i < n; i++ )
  {
    indx[i] = i;
  }

  if ( n == 1 )
  {
    indx[0] = indx[0] + base;
    return indx;
  }

  l = n / 2 + 1;
  ir = n;

  for ( ; ; )
  {
    if ( 1 < l )
    {
      l = l - 1;
      indxt = indx[l-1];
      aval[0] = a[0+indxt*2];
      aval[1] = a[1+indxt*2];
    }
    else
    {
      indxt = indx[ir-1];
      aval[0] = a[0+indxt*2];
      aval[1] = a[1+indxt*2];
      indx[ir-1] = indx[0];
      ir = ir - 1;

      if ( ir == 1 )
      {
        indx[0] = indxt;
        break;
      }
    }
    i = l;
    j = l + l;

    while ( j <= ir )
    {
      if ( j < ir )
      {
        if (   a[0+indx[j-1]*2] <  a[0+indx[j]*2] ||
             ( a[0+indx[j-1]*2] == a[0+indx[j]*2] &&
               a[1+indx[j-1]*2] <  a[1+indx[j]*2] ) )
        {
          j = j + 1;
        }
      }

      if (   aval[0] <  a[0+indx[j-1]*2] ||
           ( aval[0] == a[0+indx[j-1]*2] &&
             aval[1] <  a[1+indx[j-1]*2] ) )
      {
        indx[i-1] = indx[j-1];
        i = j;
        j = j + j;
      }
      else
      {
        j = ir + 1;
      }
    }
    indx[i-1] = indxt;
  }
//
//  Take care of the base.
//
  for ( i = 0; i < n; i++ )
  {
    indx[i] = indx[i] + base;
  }

  return indx;
}


double* r8mat_data_read (std::vector<calibrationPoint>* list, int m, int n)
{
    double * table = new double[n*m];
    for (int i=0; i<n; i++)
    {
        table[i*m] = list->at(i)._x();
        table[i*m+1] = list->at(i)._y();
    }
    return table;
}


void r8mat_header_read (std::vector<calibrationPoint>* list, int *m, int *n )
{
    *m = 2;
    *n = list->size();
    /*
    *m = file_column_count ( input_filename );

    if ( *m <= 0 )
    {
        std::cerr << "\n";
        std::cerr << "R8MAT_HEADER_READ - Fatal error!\n";
        std::cerr << "  FILE_COLUMN_COUNT failed.\n";
        *n = -1;
        return;
    }

    *n = file_row_count ( input_filename );

    if ( *n <= 0 )
    {
        std::cerr << "\n";
        std::cerr << "R8MAT_HEADER_READ - Fatal error!\n";
        std::cerr << "  FILE_ROW_COUNT failed.\n";
        return;
    }

    return;
    */
}


void r8mat_transpose_print_some ( int m, int n, double a[], int ilo, int jlo, int ihi, int jhi, std::string title )
{
# define INCX 5

  int i;
  int i2;
  int i2hi;
  int i2lo;
  int inc;
  int j;
  int j2hi;
  int j2lo;

  std::cout << "\n";
  std::cout << title << "\n";

  for ( i2lo = i4_max ( ilo, 1 ); i2lo <= i4_min ( ihi, m ); i2lo = i2lo + INCX )
  {
    i2hi = i2lo + INCX - 1;
    i2hi = i4_min ( i2hi, m );
    i2hi = i4_min ( i2hi, ihi );

    inc = i2hi + 1 - i2lo;

    std::cout << "\n";
    std::cout << "  Row: ";
    for ( i = i2lo; i <= i2hi; i++ )
    {
      std::cout << std::setw(7) << i << "       ";
    }
    std::cout << "\n";
    std::cout << "  Col\n";
    std::cout << "\n";

    j2lo = i4_max ( jlo, 1 );
    j2hi = i4_min ( jhi, n );

    for ( j = j2lo; j <= j2hi; j++ )
    {
      std::cout << std::setw(5) << j << " ";
      for ( i2 = 1; i2 <= inc; i2++ )
      {
        i = i2lo - 1 + i2;
        std::cout << std::setw(14) << a[(i-1)+(j-1)*m];
      }
      std::cout << "\n";
    }
  }

  return;
# undef INCX
}


int s_len_trim ( std::string s )
{
  int n;

  n = s.length ( );

  while ( 0 < n )
  {
    if ( s[n-1] != ' ' )
    {
      return n;
    }
    n = n - 1;
  }

  return n;
}


double s_to_r8 ( std::string s, int *lchar, bool *error )
{
  char c;
  int ihave;
  int isgn;
  int iterm;
  int jbot;
  int jsgn;
  int jtop;
  int nchar;
  int ndig;
  double r;
  double rbot;
  double rexp;
  double rtop;
  char TAB = 9;

  nchar = s_len_trim ( s );
  *error = false;
  r = 0.0;
  *lchar = -1;
  isgn = 1;
  rtop = 0.0;
  rbot = 1.0;
  jsgn = 1;
  jtop = 0;
  jbot = 1;
  ihave = 1;
  iterm = 0;

  for ( ; ; )
  {
    c = s[*lchar+1];
    *lchar = *lchar + 1;
//
//  Blank or TAB character.
//
    if ( c == ' ' || c == TAB )
    {
      if ( ihave == 2 )
      {
      }
      else if ( ihave == 6 || ihave == 7 )
      {
        iterm = 1;
      }
      else if ( 1 < ihave )
      {
        ihave = 11;
      }
    }
//
//  Comma.
//
    else if ( c == ',' || c == ';' )
    {
      if ( ihave != 1 )
      {
        iterm = 1;
        ihave = 12;
        *lchar = *lchar + 1;
      }
    }
//
//  Minus sign.
//
    else if ( c == '-' )
    {
      if ( ihave == 1 )
      {
        ihave = 2;
        isgn = -1;
      }
      else if ( ihave == 6 )
      {
        ihave = 7;
        jsgn = -1;
      }
      else
      {
        iterm = 1;
      }
    }
//
//  Plus sign.
//
    else if ( c == '+' )
    {
      if ( ihave == 1 )
      {
        ihave = 2;
      }
      else if ( ihave == 6 )
      {
        ihave = 7;
      }
      else
      {
        iterm = 1;
      }
    }
//
//  Decimal point.
//
    else if ( c == '.' )
    {
      if ( ihave < 4 )
      {
        ihave = 4;
      }
      else if ( 6 <= ihave && ihave <= 8 )
      {
        ihave = 9;
      }
      else
      {
        iterm = 1;
      }
    }
//
//  Exponent marker.
//
    else if ( ch_eqi ( c, 'E' ) || ch_eqi ( c, 'D' ) )
    {
      if ( ihave < 6 )
      {
        ihave = 6;
      }
      else
      {
        iterm = 1;
      }
    }
//
//  Digit.
//
    else if ( ihave < 11 && '0' <= c && c <= '9' )
    {
      if ( ihave <= 2 )
      {
        ihave = 3;
      }
      else if ( ihave == 4 )
      {
        ihave = 5;
      }
      else if ( ihave == 6 || ihave == 7 )
      {
        ihave = 8;
      }
      else if ( ihave == 9 )
      {
        ihave = 10;
      }

      ndig = ch_to_digit ( c );

      if ( ihave == 3 )
      {
        rtop = 10.0 * rtop + ( double ) ndig;
      }
      else if ( ihave == 5 )
      {
        rtop = 10.0 * rtop + ( double ) ndig;
        rbot = 10.0 * rbot;
      }
      else if ( ihave == 8 )
      {
        jtop = 10 * jtop + ndig;
      }
      else if ( ihave == 10 )
      {
        jtop = 10 * jtop + ndig;
        jbot = 10 * jbot;
      }

    }
//
//  Anything else is regarded as a terminator.
//
    else
    {
      iterm = 1;
    }
//
//  If we haven't seen a terminator, and we haven't examined the
//  entire std::string, go get the next character.
//
    if ( iterm == 1 || nchar <= *lchar + 1 )
    {
      break;
    }

  }
//
//  If we haven't seen a terminator, and we have examined the
//  entire std::string, then we're done, and LCHAR is equal to NCHAR.
//
  if ( iterm != 1 && (*lchar) + 1 == nchar )
  {
    *lchar = nchar;
  }
//
//  Number seems to have terminated.  Have we got a legal number?
//  Not if we terminated in states 1, 2, 6 or 7!
//
  if ( ihave == 1 || ihave == 2 || ihave == 6 || ihave == 7 )
  {
    *error = true;
    return r;
  }
//
//  Number seems OK.  Form it.
//
  if ( jtop == 0 )
  {
    rexp = 1.0;
  }
  else
  {
    if ( jbot == 1 )
    {
      rexp = pow ( 10.0, jsgn * jtop );
    }
    else
    {
      rexp = jsgn * jtop;
      rexp = rexp / jbot;
      rexp = pow ( 10.0, rexp );
    }

  }

  r = isgn * rexp * rtop / rbot;

  return r;
}


bool s_to_r8vec ( std::string s, int n, double rvec[] )
{
  int begin;
  bool error;
  int i;
  int lchar;
  int length;

  begin = 0;
  length = s.length ( );
  error = 0;

  for ( i = 0; i < n; i++ )
  {
    rvec[i] = s_to_r8 ( s.substr(begin,length), &lchar, &error );

    if ( error )
    {
      return error;
    }
    begin = begin + lchar;
    length = length - lchar;
  }

  return error;
}


int s_word_count ( std::string s )
{
  bool blank;
  int char_count;
  int i;
  int word_count;

  word_count = 0;
  blank = true;

  char_count = s.length ( );

  for ( i = 0; i < char_count; i++ )
  {
    if ( isspace ( s[i] ) )
    {
      blank = true;
    }
    else if ( blank )
    {
      word_count = word_count + 1;
      blank = false;
    }
  }

  return word_count;
}


int swapec ( int i, int *top, int *btri, int *bedg, int point_num, double point_xy[], int tri_num, int tri_vert[], int tri_nabe[], int stack[] )
{
  int a;
  int b;
  int c;
  int e;
  int ee;
  int em1;
  int ep1;
  int f;
  int fm1;
  int fp1;
  int l;
  int r;
  int s;
  int swap;
  int t;
  int tt;
  int u;
  double x;
  double y;
//
//  Determine whether triangles in stack are Delaunay, and swap
//  diagonal edge of convex quadrilateral if not.
//
  x = point_xy[2*(i-1)+0];
  y = point_xy[2*(i-1)+1];

  for ( ; ; )
  {
    if ( *top <= 0 )
    {
      break;
    }

    t = stack[(*top)-1];
    *top = *top - 1;

    if ( tri_vert[3*(t-1)+0] == i )
    {
      e = 2;
      b = tri_vert[3*(t-1)+2];
    }
    else if ( tri_vert[3*(t-1)+1] == i )
    {
      e = 3;
      b = tri_vert[3*(t-1)+0];
    }
    else
    {
      e = 1;
      b = tri_vert[3*(t-1)+1];
    }

    a = tri_vert[3*(t-1)+e-1];
    u = tri_nabe[3*(t-1)+e-1];

    if ( tri_nabe[3*(u-1)+0] == t )
    {
      f = 1;
      c = tri_vert[3*(u-1)+2];
    }
    else if ( tri_nabe[3*(u-1)+1] == t )
    {
      f = 2;
      c = tri_vert[3*(u-1)+0];
    }
    else
    {
      f = 3;
      c = tri_vert[3*(u-1)+1];
    }

    swap = diaedg ( x, y,
      point_xy[2*(a-1)+0], point_xy[2*(a-1)+1],
      point_xy[2*(c-1)+0], point_xy[2*(c-1)+1],
      point_xy[2*(b-1)+0], point_xy[2*(b-1)+1] );

    if ( swap == 1 )
    {
      em1 = i4_wrap ( e - 1, 1, 3 );
      ep1 = i4_wrap ( e + 1, 1, 3 );
      fm1 = i4_wrap ( f - 1, 1, 3 );
      fp1 = i4_wrap ( f + 1, 1, 3 );

      tri_vert[3*(t-1)+ep1-1] = c;
      tri_vert[3*(u-1)+fp1-1] = i;
      r = tri_nabe[3*(t-1)+ep1-1];
      s = tri_nabe[3*(u-1)+fp1-1];
      tri_nabe[3*(t-1)+ep1-1] = u;
      tri_nabe[3*(u-1)+fp1-1] = t;
      tri_nabe[3*(t-1)+e-1] = s;
      tri_nabe[3*(u-1)+f-1] = r;

      if ( 0 < tri_nabe[3*(u-1)+fm1-1] )
      {
        *top = *top + 1;
        stack[(*top)-1] = u;
      }

      if ( 0 < s )
      {
        if ( tri_nabe[3*(s-1)+0] == u )
        {
          tri_nabe[3*(s-1)+0] = t;
        }
        else if ( tri_nabe[3*(s-1)+1] == u )
        {
          tri_nabe[3*(s-1)+1] = t;
        }
        else
        {
          tri_nabe[3*(s-1)+2] = t;
        }

        *top = *top + 1;

        if ( point_num < *top )
        {
          return 8;
        }

        stack[(*top)-1] = t;
      }
      else
      {
        if ( u == *btri && fp1 == *bedg )
        {
          *btri = t;
          *bedg = e;
        }

        l = - ( 3 * t + e - 1 );
        tt = t;
        ee = em1;

        while ( 0 < tri_nabe[3*(tt-1)+ee-1] )
        {
          tt = tri_nabe[3*(tt-1)+ee-1];

          if ( tri_vert[3*(tt-1)+0] == a )
          {
            ee = 3;
          }
          else if ( tri_vert[3*(tt-1)+1] == a )
          {
            ee = 1;
          }
          else
          {
            ee = 2;
          }
        }
        tri_nabe[3*(tt-1)+ee-1] = l;
      }

      if ( 0 < r )
      {
        if ( tri_nabe[3*(r-1)+0] == t )
        {
          tri_nabe[3*(r-1)+0] = u;
        }
        else if ( tri_nabe[3*(r-1)+1] == t )
        {
          tri_nabe[3*(r-1)+1] = u;
        }
        else
        {
          tri_nabe[3*(r-1)+2] = u;
        }
      }
      else
      {
        if ( t == *btri && ep1 == *bedg )
        {
          *btri = u;
          *bedg = f;
        }

        l = - ( 3 * u + f - 1 );
        tt = u;
        ee = fm1;

        while ( 0 < tri_nabe[3*(tt-1)+ee-1] )
        {
          tt = tri_nabe[3*(tt-1)+ee-1];

          if ( tri_vert[3*(tt-1)+0] == b )
          {
            ee = 3;
          }
          else if ( tri_vert[3*(tt-1)+1] == b )
          {
            ee = 1;
          }
          else
          {
            ee = 2;
          }

        }
        tri_nabe[3*(tt-1)+ee-1] = l;
      }
    }
  }

  return 0;
}


void timestamp ( )
{
# define TIME_SIZE 29

  static char time_buffer[TIME_SIZE];
  const struct tm *tm;
  size_t len;
  time_t now;

  now = time ( NULL );
  tm = localtime ( &now );

  len = strftime ( time_buffer, TIME_SIZE, "%d %B %Y %I:%M:%S %p", tm );

  if ( len != 0 )
  {
    std::cout << time_buffer << "\n";
  }

  return;
# undef TIME_SIZE
}


void vbedg ( double x, double y, int point_num, double point_xy[], int tri_num, int tri_vert[], int tri_nabe[], int *ltri, int *ledg, int *rtri, int *redg )
{
  int a;
  double ax;
  double ay;
  int b;
  double bx;
  double by;
  bool done;
  int e;
  int l;
  int lr;
  int t;
//
//  Find the rightmost visible boundary edge using links, then possibly
//  leftmost visible boundary edge using triangle neighbor information.
//
  if ( *ltri == 0 )
  {
    done = false;
    *ltri = *rtri;
    *ledg = *redg;
  }
  else
  {
    done = true;
  }

  for ( ; ; )
  {
    l = -tri_nabe[3*((*rtri)-1)+(*redg)-1];
    t = l / 3;
    e = 1 + l % 3;
    a = tri_vert[3*(t-1)+e-1];

    if ( e <= 2 )
    {
      b = tri_vert[3*(t-1)+e];
    }
    else
    {
      b = tri_vert[3*(t-1)+0];
    }

    ax = point_xy[2*(a-1)+0];
    ay = point_xy[2*(a-1)+1];

    bx = point_xy[2*(b-1)+0];
    by = point_xy[2*(b-1)+1];

    lr = lrline ( x, y, ax, ay, bx, by, 0.0 );

    if ( lr <= 0 )
    {
      break;
    }

    *rtri = t;
    *redg = e;

  }

  if ( done )
  {
    return;
  }

  t = *ltri;
  e = *ledg;

  for ( ; ; )
  {
    b = tri_vert[3*(t-1)+e-1];
    e = i4_wrap ( e-1, 1, 3 );

    while ( 0 < tri_nabe[3*(t-1)+e-1] )
    {
      t = tri_nabe[3*(t-1)+e-1];

      if ( tri_vert[3*(t-1)+0] == b )
      {
        e = 3;
      }
      else if ( tri_vert[3*(t-1)+1] == b )
      {
        e = 1;
      }
      else
      {
        e = 2;
      }

    }

    a = tri_vert[3*(t-1)+e-1];
    ax = point_xy[2*(a-1)+0];
    ay = point_xy[2*(a-1)+1];

    bx = point_xy[2*(b-1)+0];
    by = point_xy[2*(b-1)+1];

    lr = lrline ( x, y, ax, ay, bx, by, 0.0 );

    if ( lr <= 0 )
    {
      break;
    }

  }

  *ltri = t;
  *ledg = e;

  return;
}
