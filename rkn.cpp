/***************************************************************************************************************\
*   Моделирование системы уравнений:                                                                            *
*                                                                                                               *
*   i\frac{\partial^2 A}{\partial x^2} + \frac{\partial A}{\partial\tau} + \frac{\partial A}{\partial z} = J    *
*   \frac{\partial^2\theta}{\partial z^2} = Re\left(A e^{i\theta}\right)                                        *
*   A(\tau)|_{z=0} = KA(\tau)|_{z=L}                                                                            *
*   \theta|_{z=0} = \theta_0\in [0, 2\pi]                                                                       *
*   \frac{\partial\theta}{\partial z} = \Delta                                                                  *
*                                                                                                               *
*   методом Рунге-Кутты-Нистрема                                                                                *
\***************************************************************************************************************/

#include <fstream>
#include <iostream>
#include <QDebug>

#include "dialog.h"
#include "rkn.h"
#include <QDir>
#include <QMutex>
#include <QMutexLocker>

QMutex mx;

Rkn::Rkn(QObject *parent) : QObject(parent), ic(0.0, 1.0)
{
   Dialog *d = new Dialog(geth(), getL(), getNe(), getAr(), getAi(), getdelta());

   d->exec();

   ifstream in("input.dat");
   if (!in) {
      qDebug() << "Error opening file input.dat";
      exit(1);
   }

   //****************************************************************************************//
   //							 Читаем данные из файла input.dat
   //****************************************************************************************//
   in >> h >> L >> Ne >> Ar >> delta >> phase_space >> draw_trajectories;
   //========================================================================================//
   //						   / Читаем данные из файла input.dat
   //========================================================================================//

   //****************************************************************************************//
   //							 Печатаем в консоли данные из файла input.dat
   //****************************************************************************************//
   cout << "h = " << h << "\nL = " << L << "\nNe = " << Ne << "\ndelta = " << delta << endl << endl;
   //****************************************************************************************//
   //							 Печатаем в консоли данные из файла input.dat
   //****************************************************************************************//

   NZ = int(L / h) + 1;
   hth = 2 * M_PI / Ne;

   cout << "NZ = " << NZ << "\nh = " << hth << endl;

   //****************************************************************************************//
   //							 Массивы для A(z), th(z,th0), dthdz(z,th0)
   //****************************************************************************************//
   p = new complex<double> *[Ne];
   for (int i = 0; i < Ne; i++) {
      p[i] = new complex<double>[NZ];
   }
   z = new double[NZ];
   s1 = new complex<double>[Ne];
   s2 = new complex<double>[Ne];
   s3 = new complex<double>[Ne];
   s4 = new complex<double>[Ne];

   th = new double *[NZ];
   dthdz = new double *[NZ];
   for (int i = 0; i < NZ; i++) {
      th[i] = new double[Ne];
      dthdz[i] = new double[Ne];
   }
   F0 = new double[Ne];
   A = new complex<double>[NZ];

   //========================================================================================//
   //						   / Массивы для A(z), th(z,th0), dthdz(z,th0)
   //========================================================================================//

   for (int i = 0; i < NZ; i++) // формируем вектор пространственных координат
   {
      z[i] = i * h;
   }

   calc_A(A, z, Ar, Ai);
}

bool Rkn::calculating() const
{
   return m_calculating;
}

void Rkn::setCalculating(bool calculating)
{
   if (m_calculating == calculating)
      return;

   m_calculating = calculating;
   emit calculatingChanged(calculating);

   //    qDebug() << "m_calculating = " << m_calculating;
}

bool Rkn::stop() const
{
   return m_calculating;
}

void Rkn::setStop(bool stop)
{
   if (m_stop == stop)
      return;

   m_stop = stop;
   emit stopChanged(stop);

   //    qDebug() << "m_calculating = " << m_calculating;
}

void Rkn::calculate()
{
   //   QMutexLocker ml(&mx);
   //****************************************************************************************//
   //						     Начальные условия
   //****************************************************************************************//

   for (int k = 0; k < Ne; k++) {
      //      th[0][k] = hth * k;
      //      th[0][k] = hth * k - M_PI / 2.0;
      //      dthdz[0][k] = delta;
      p[k][0] = exp(ic * hth * double(k));
      for (int i = 1; i < NZ; i++) {
         //         th[i][k] = 0.0;
         //         dthdz[i][k] = 0.0;
         p[k][i] = 0.0;
      }
   }  

   for (int k = 0; k < Ne; k++) {
      qDebug() << real(p[k][0]);
   }
   qDebug() << "h = " << h;
   qDebug() << "hth = " << h;

   //   ofstream f;
   //   f.open("test.dat");
   //   for (int i = 0; i < Ne; i++) {
   //      f << i << ' ' << th[0][i] << '\n';
   //   }
   //   f.close();

   //========================================================================================//
   //						   / Начальные условия
   //========================================================================================//

   complex<double> v;
   double pre, pim, absp;
   for (int i = 0; i < NZ - 1; i++) {
      for (int k = 0; k < Ne; k++) {
         v = p[k][i];
         s1[k] = F(z[i], p[k][i], A[i]);
         s2[k] = F(z[i] + h / 2.0, v + h * s1[k] / 2.0, A[i]);

         s3[k] = F(z[i] + h / 2.0, v + h * s2[k] / 2.0, A[i]);

         s4[k] = F(z[i] + h, v + h * s3[k], A[i]);
         p[k][i + 1] = v + h * (s1[k] + 2.0 * s2[k] + 2.0 * s3[k] + s4[k]) / 6.0;

         pre = real(p[k][i + 1]);
         pim = imag(p[k][i + 1]);
         if (pre < premin)
            premin = pre;
         if (pre > premax)
            premax = pre;
         if (pim < pimmin)
            pimmin = pim;
         if (pim > pimmax)
            pimmax = pim;

         absp = abs(p[k][i + 1]);
         if (absp < abspmin)
            abspmin = absp;
         if (absp > abspmax)
            abspmax = absp;
      }

      it = i + 1;
      emit paintSignal();
      QMutexLocker ml(&mx);
   }

   if (m_stop) {
      while (m_calculating) {
      }
      emit finished();
   }

   cout << endl;

   //========================================================================================//
   //						   / Main part
   //========================================================================================//

   while (m_calculating) {
   }
   emit finished();
}

//****************************************************************************************//
//							 Functions
//****************************************************************************************//

inline complex<double> Rkn::F(double z, complex<double> p, complex<double> A)
{
   return ic * (A - (delta + abs(p) * abs(p) - 1.0) * p);
}

inline void Rkn::calc_A(complex<double> *A, double *z, double Ar, double Ai)
{  
   for (int i = 0; i < NZ; i++) {
      A[i] = sqrt(Ar * Ar + Ai * Ai)
             * exp(-3.0 * (((z[i] - L / 2.0) / (L / 2.0)) * ((z[i] - L / 2.0) / (L / 2.0))));

      if (abs(A[i]) < Amin)
         Amin = abs(A[i]);
      if (abs(A[i]) > Amax)
         Amax = abs(A[i]);
   }

   ofstream f;
   f.open("test.dat");
   for (int i = 0; i < NZ; i++) {
      f << i << " " << abs(A[i]) << endl;
   }
   f.close();
}
//========================================================================================//
//						   / Functions
//========================================================================================//