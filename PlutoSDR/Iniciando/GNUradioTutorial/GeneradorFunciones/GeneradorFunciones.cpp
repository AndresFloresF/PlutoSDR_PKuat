/********************
GNU Radio C++ Flow Graph Source File

Title: Not titled yet
Author: andresflores
GNU Radio version: v3.8.5.0-6-g57bd109d
********************/

#include "GeneradorFunciones.hpp"
using namespace gr;


GeneradorFunciones::GeneradorFunciones () : QWidget() {
    this->setWindowTitle("Not titled yet");
    // check_set_qss
    // set icon
    this->top_scroll_layout = new QVBoxLayout();
    this->setLayout(this->top_scroll_layout);
    this->top_scroll = new QScrollArea();
    this->top_scroll->setFrameStyle(QFrame::NoFrame);
    this->top_scroll_layout->addWidget(this->top_scroll);
    this->top_scroll->setWidgetResizable(true);
    this->top_widget = new QWidget();
    this->top_scroll->setWidget(this->top_widget);
    this->top_layout = new QVBoxLayout(this->top_widget);
    this->top_grid_layout = new QGridLayout();
    this->top_layout->addLayout(this->top_grid_layout);

    this->settings = new QSettings("GNU Radio", "GeneradorFunciones");




    this->tb = gr::make_top_block("Not titled yet");


// Blocks:
    {
        this->qtgui::freq_sink_x_0 = qtgui:::freq_sink_c::make(
            1024, // size
            filter::firdes::WIN_BLACKMAN_hARRIS, // wintype
            0, // fc
            samp_rate, // bw
            "", // name
            1 // nconnections
        );

        std::string labels[10] = {"", "", "", "", "",
            "", "", "", "", ""};
        int widths[10] = {1, 1, 1, 1, 1,
            1, 1, 1, 1, 1};
        std::string colors[10] = {"blue", "red", "green", "black", "cyan",
            "magenta", "yellow", "dark red", "dark green", "dark blue"};
        double alphas[10] = {1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0};

        QWidget* _qtgui::freq_sink_x_0_win;

        this->qtgui::freq_sink_x_0->set_update_time(0.10);
        this->qtgui::freq_sink_x_0->set_y_axis(-140, 10);
        this->qtgui::freq_sink_x_0->set_y_label("Relative Gain", "dB");
        this->qtgui::freq_sink_x_0->set_trigger_mode(qtgui::TRIG_MODE_FREE, 0.0, 0, "");
        this->qtgui::freq_sink_x_0->enable_autoscale(false);
        this->qtgui::freq_sink_x_0->enable_grid(false);
        this->qtgui::freq_sink_x_0->set_fft_average(1.0);
        this->qtgui::freq_sink_x_0->enable_axis_labels(true);
        this->qtgui::freq_sink_x_0->enable_control_panel(false);

        if (!true) {
            this->qtgui::freq_sink_x_0->disable_legend(); // if (!legend)
        }

        /* C++ doesn't have this
        if ("complex" == "float" or "complex" == "msg_float") {
            this->qtgui::freq_sink_x_0->set_plot_pos_half(not true);
        }*/

        for (int i = 0; i < 1; i++) {
            if (sizeof(labels[i]) == 0) {
                this->qtgui::freq_sink_x_0->set_line_label(i, "Data " + std::to_string(i));
            } else {
                this->qtgui::freq_sink_x_0->set_line_label(i, labels[i]);
            }
            this->qtgui::freq_sink_x_0->set_line_width(i, widths[i]);
            this->qtgui::freq_sink_x_0->set_line_color(i, colors[i]);
            this->qtgui::freq_sink_x_0->set_line_alpha(i, alphas[i]);
        }

        _qtgui::freq_sink_x_0_win = this->qtgui::freq_sink_x_0->qwidget();
        this->top_layout->addWidget(_qtgui::freq_sink_x_0_win);
    }
    {
        this->blocks_throttle_0 = blocks::throttle::make(sizeof(gr_complex)*1, samp_rate, true);
    }
    {
        this->analog_sig_source_x_0 = analog::sig_source_c::make(samp_rate, analog::GR_SQR_WAVE, 1000, 1, 0,0);
    }

// Connections:
    this->tb->hier_block2::connect(this->analog_sig_source_x_0, 0, this->blocks_throttle_0, 0);
    this->tb->hier_block2::connect(this->blocks_throttle_0, 0, this->qtgui_freq_sink_x_0, 0);
}

GeneradorFunciones::~GeneradorFunciones () {
}

// Callbacks:
int GeneradorFunciones::get_samp_rate () const {
    return this->samp_rate;
}

void GeneradorFunciones::set_samp_rate (int samp_rate) {
    this->samp_rate = samp_rate;
    this->analog_sig_source_x_0->set_sampling_freq(this->samp_rate);
    this->blocks_throttle_0->set_sample_rate(this->samp_rate);
    this->qtgui_freq_sink_x_0->set_frequency_range(0, this->samp_rate);
}


int main (int argc, char **argv) {

    QApplication app(argc, argv);

    GeneradorFunciones* top_block = new GeneradorFunciones();

    top_block->tb->start();
    top_block->show();
    app.exec();


    return 0;
}
#include "moc_GeneradorFunciones.cpp"
