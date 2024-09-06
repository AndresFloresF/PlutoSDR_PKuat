#ifndef GENERADORFUNCIONES_HPP
#define GENERADORFUNCIONES_HPP
/********************
GNU Radio C++ Flow Graph Header File

Title: Not titled yet
Author: andresflores
GNU Radio version: v3.8.5.0-6-g57bd109d
********************/

/********************
** Create includes
********************/
#include <gnuradio/top_block.h>
#include <gnuradio/analog/sig_source.h>
#include <gnuradio/blocks/throttle.h>
#include <gnuradio/qtgui/freq_sink_c.h>
#include <gnuradio/filter/firdes.h>

#include <QVBoxLayout>
#include <QScrollArea>
#include <QWidget>
#include <QGridLayout>
#include <QSettings>


using namespace gr;



class GeneradorFunciones : public QWidget {
    Q_OBJECT

private:
    QVBoxLayout *top_scroll_layout;
    QScrollArea *top_scroll;
    QWidget *top_widget;
    QVBoxLayout *top_layout;
    QGridLayout *top_grid_layout;
    QSettings *settings;


    qtgui::freq_sink_c::sptr qtgui_freq_sink_x_0;
    blocks::throttle::sptr blocks_throttle_0;
    analog::sig_source_c::sptr analog_sig_source_x_0;


// Variables:
    int samp_rate = 32000;

public:
    top_block_sptr tb;
    GeneradorFunciones();
    ~GeneradorFunciones();

    int get_samp_rate () const;
    void set_samp_rate(int samp_rate);

};


#endif

