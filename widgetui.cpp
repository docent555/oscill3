#include "widgetui.h"
#include "ui_widgetui.h"

#include "rkn.h"

#include <fstream>
#include <QDebug>
#include <QGridLayout>

Widgetui::Widgetui(Rkn *r, QWidget *parent)
    : QWidget(parent), ui(new Ui::Widgetui), m_listCount(3), m_valueMax(10), m_valueCount(7)
{
   ui->setupUi(this);

   z = r->getz();
   nz = r->getnz();
   p = r->get_p();
   it = r->getit();
   ne = r->getNe();
   phase_space = r->get_phase_space();
   draw_trajectories = r->get_draw_trajectories();
   A = r->getA();

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

   sAr = "Амплитуда поля: " + sAr.setNum(real(A[0]));
   sNth = "Число электронов: " + sNth.setNum(Ne);
   sdelta = "Расстройка синхронизма: " + sdelta.setNum(delta);
   sL = "Длина системы: " + sL.setNum(L);
   sh = "Шаг: " + sh.setNum(h);

   ui->label_Ar->setText(sAr);
   ui->label_L->setText(sL);
   ui->label_Ne->setText(sNth);
   ui->label_delta->setText(sdelta);
   ui->label_h->setText(sh);

   qDebug() << "In Widgetui A = " << A;

   QSize min(300, 300);
   QSize max(900, 900);
   if (phase_space == 0) {
      ymin = r->get_abspmin();
      ymax = r->get_abspmax();      
      *xmin = 0;
      xmax = &z[nz - 1];      
      chart = new QChart();
      chart = createScatterChart_trj();
      ui->pView->setChart(chart);
      ui->pView->setMinimumSize(min);
      ui->pView->setMaximumSize(max);
   } else {
      ymin = r->get_pimmin();
      ymax = r->get_pimmax();
      xmin = r->get_premin();
      xmax = r->get_premax();
      chart = new QChart();
      chart = createScatterChart_phs();
      ui->pView->setChart(chart);
      ui->pView->setMinimumSize(min);
      ui->pView->setMaximumSize(max);
   }

   qDebug() << "Перед созданием профиля";

   ymin_prof = r->get_Amin();
   ymax_prof = r->get_Amax();
   chart_prof = new QChart();
   chart_prof = createMixedChart_profile();
   ui->profView->setChart(chart_prof);

   m_charts << ui->pView << ui->profView;

   updateUI();

   connect(r, SIGNAL(paintSignal()), this, SLOT(paintGraph()), Qt::BlockingQueuedConnection);

   init_paintGraph();
}

Widgetui::~Widgetui()
{
   delete ui;
}

QChart *Widgetui::createMixedChart_profile()
{
   QChart *chart = new QChart();
   chart->setTitle("Траектории");
   chart->setTheme(QChart::ChartThemeDark);
   QFont font = chart->titleFont();
   font.setPointSize(10);
   chart->setTitleFont(font);

   series_prof_line.append(new QLineSeries());
   series_prof_line[0]->setUseOpenGL(true);

   series_prof_scat.append(new QScatterSeries());
   series_prof_scat[0]->setUseOpenGL(true);

   xAxis_prof = new QValueAxis; // Ось X
   xAxis_prof->setRange(0, L);
   xAxis_prof->setTitleText(tr("z")); // Название оси X
   font = xAxis_prof->titleFont();
   font.setPointSize(10);
   xAxis_prof->setTitleFont(font);
   font = xAxis_prof->labelsFont();
   font.setPointSize(8);
   xAxis_prof->setLabelsFont(font);

   qDebug() << "Ссоздание серии для профиля";

   yAxis_prof = new QValueAxis; // Ось Y
   yAxis_prof->setRange(-0.2, 1.2);
   font = xAxis_prof->titleFont();
   font.setPointSize(10);
   yAxis_prof->setTitleFont(font);
   font = xAxis_prof->labelsFont();
   font.setPointSize(8);
   yAxis_prof->setLabelsFont(font);

   chart->addAxis(xAxis_prof, Qt::AlignBottom);
   chart->addAxis(yAxis_prof, Qt::AlignLeft);

   chart->addSeries(series_prof_line[0]);
   series_prof_line[0]->attachAxis(xAxis_prof);
   series_prof_line[0]->attachAxis(yAxis_prof);

   chart->addSeries(series_prof_scat[0]);
   series_prof_scat[0]->attachAxis(xAxis_prof);
   series_prof_scat[0]->attachAxis(yAxis_prof);

   qDebug() << "Конец создания серии для профиля";

   return chart;
}

QChart *Widgetui::createScatterChart_trj()
{
   QChart *chart = new QChart();
   chart->setTitle("Траектории");
   chart->setTheme(QChart::ChartThemeDark);
   QFont font = chart->titleFont();
   font.setPointSize(16);
   chart->setTitleFont(font);

   for (int i = 0; i < ne; i++) {
      series.append(new QScatterSeries());
      series[i]->setUseOpenGL(true);
   }

   xAxis = new QValueAxis; // Ось X
                           //    xAxis->setRange(0, z[nz - 1]);
   xAxis->setRange(*xmin, *xmax);
   xAxis->setTitleText(tr("z")); // Название оси X
   //    xAxis->setTitleBrush(Qt::black); // Цвет названия
   //    xAxis->setLabelsColor(Qt::black); // Цвет элементов оси
   font = xAxis->titleFont();
   font.setPointSize(16);
   xAxis->setTitleFont(font);
   font = xAxis->labelsFont();
   font.setPointSize(12);
   xAxis->setLabelsFont(font);

   yAxis = new QValueAxis;         // Ось Y
   yAxis->setRange(*ymin, *ymax);  // Диапазон от -20 до +20 Вольт
   yAxis->setTitleText(tr("|p|")); // Название оси Y
   //    yAxis->setTitleBrush(Qt::black); // Цвет названия
   //    yAxis->setLabelsColor(Qt::black); // Цвет элементов оси
   font = xAxis->titleFont();
   font.setPointSize(16);
   yAxis->setTitleFont(font);
   font = xAxis->labelsFont();
   font.setPointSize(12);
   yAxis->setLabelsFont(font);

   chart->addAxis(xAxis, Qt::AlignBottom);
   chart->addAxis(yAxis, Qt::AlignLeft);

   for (int i = 0; i < ne; ++i) {
      chart->addSeries(series[i]);
      //    chart->setAxisX(xAxis, serieskpd); // Назначить ось xAxis, осью X для diagramA
      series[i]->attachAxis(xAxis);
      //    chart->setAxisY(yAxis, serieskpd); // Назначить ось yAxis, осью Y для diagramA
      series[i]->attachAxis(yAxis);
   }

   return chart;
}

QChart *Widgetui::createScatterChart_phs()
{
   QChart *chart = new QChart();
   //   chart->setTitle("Фазовая плоскость");
   chart->setTheme(QChart::ChartThemeDark);
   QFont font = chart->titleFont();
   font.setPointSize(16);
   chart->setTitleFont(font);

   for (int i = 0; i < ne; i++) {
      series.append(new QScatterSeries());
      series[i]->setUseOpenGL(true);
   }

   xAxis = new QValueAxis; // Ось X
   xAxis->setRange(*xmin, *xmax);
   xAxis->setTitleText(tr("Re(p)")); // Название оси X
   //    xAxis->setTitleBrush(Qt::black); // Цвет названия
   //    xAxis->setLabelsColor(Qt::black); // Цвет элементов оси
   font = xAxis->titleFont();
   font.setPointSize(16);
   xAxis->setTitleFont(font);
   font = xAxis->labelsFont();
   font.setPointSize(12);
   xAxis->setLabelsFont(font);

   yAxis = new QValueAxis;           // Ось Y
   yAxis->setRange(*ymin, *ymax);    // Диапазон от -20 до +20 Вольт
   yAxis->setTitleText(tr("Im(p)")); // Название оси Y
   //    yAxis->setTitleBrush(Qt::black); // Цвет названия
   //    yAxis->setLabelsColor(Qt::black); // Цвет элементов оси
   font = xAxis->titleFont();
   font.setPointSize(16);
   yAxis->setTitleFont(font);
   font = xAxis->labelsFont();
   font.setPointSize(12);
   yAxis->setLabelsFont(font);

   chart->addAxis(xAxis, Qt::AlignBottom);
   chart->addAxis(yAxis, Qt::AlignLeft);

   for (int i = 0; i < ne; ++i) {
      chart->addSeries(series[i]);
      //    chart->setAxisX(xAxis, serieskpd); // Назначить ось xAxis, осью X для diagramA
      series[i]->attachAxis(xAxis);
      //    chart->setAxisY(yAxis, serieskpd); // Назначить ось yAxis, осью Y для diagramA
      series[i]->attachAxis(yAxis);
   }

   return chart;
}

void Widgetui::updateUI()
{
   const auto charts = m_charts;

   bool checked = true;
   for (QChartView *chart : charts)
      chart->setRenderHint(QPainter::Antialiasing, checked);

   for (QChartView *chartView : charts) {
      chartView->chart()->legend()->hide();
   }
}

void Widgetui::paintGraph()
{
   static int j;
   j = *it;
   QColor green(Qt::green);
   QColor red(Qt::red);

   if (phase_space == 0) {
      yAxis->setRange((*ymin) - 0.2, (*ymax) + 0.2);

      for (int i = 0; i < ne; i++) {
         if (draw_trajectories == 0)
            series[i]->clear();
         series[i]->setBrush(green);
         series[i]->append(z[j], abs(p[i][j]));
      }      
   } else {
      xAxis->setRange((*xmin) - 0.2, (*xmax) + 0.2);
      yAxis->setRange((*ymin) - 0.2, (*ymax) + 0.2);

      for (int i = 0; i < ne; i++) {
         if (draw_trajectories == 0)
            series[i]->clear();
         series[i]->setBrush(green);
         series[i]->append(real(p[i][j]), imag(p[i][j]));
      }
   }
   series_prof_scat[0]->clear();
   series_prof_scat[0]->setBrush(red);
   series_prof_scat[0]->append(z[j], abs(A[j]));
   sAr = "Амплитуда поля: " + sAr.setNum(real(A[j]));
   ui->label_Ar->setText(sAr);
}

void Widgetui::init_paintGraph()
{
   static int j;
   j = 0;
   QColor green(Qt::green);
   QColor red(Qt::red);

   if (phase_space == 0) {
      yAxis->setRange((*ymin) - 0.2, (*ymax) + 0.2);

      for (int i = 0; i < ne; i++) {
         if (draw_trajectories == 0)
            series[i]->clear();
         series[i]->setBrush(green);
         series[i]->append(z[j], abs(p[i][j]));
      }
      for (int i = 0; i < nz; i++)
         series_prof_line[0]->append(z[i], abs(A[i]));
   } else {
      xAxis->setRange((*xmin) - 0.2, (*xmax) + 0.2);
      yAxis->setRange((*ymin) - 0.2, (*ymax) + 0.2);

      for (int i = 0; i < ne; i++) {
         if (draw_trajectories == 0)
            series[i]->clear();
         series[i]->setBrush(green);
         series[i]->append(real(p[i][j]), imag(p[i][j]));
      }
      for (int i = 0; i < nz; i++)
         series_prof_line[0]->append(z[i], abs(A[i]));
   }
}
