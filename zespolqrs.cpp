#include <iostream>
#include <wfdb/wfdb.h>
#include <wfdb/ecgcodes.h>
#include <vector>

class qrs_analyzer
{
public:
    qrs_analyzer() : file_name(), number_of_signals(0), slope_criterion_min(0),
        annotation_info(), result_annotation(), slope_criterion_max(0), slope_criterion(0),
        slope_number(0), number_of_160ms_intervals(0), max_time(0), number_of_200ms_intervals(0),
        number_of_2s_intervals(0), filter(0), time(0), qtime(0), sign(0), maxslope(0)
    {
        read_file_name();
        get_number_of_signals();
        fill_signal_info();
        fill_result_annotation();
    }

    ~qrs_analyzer()
    {
        delete[] info;
        delete[] samples;
        wfdbquit();
    }

    void execute()
    {
        set_samples_frequency();
        get_annotation();
        getsc_min_max();
        get_sample_intervals();
        init_samples();
        qrs_detection_algorithm();
    }
private:
    void read_file_name()
    {
        std::cout << "Provide file name with record:\n";
        std::cin >> file_name; 
    }

    void get_number_of_signals()
    {
        number_of_signals = isigopen(const_cast<char*>(file_name.c_str()), NULL, 0);
        if(number_of_signals < 2)
        {
            exit(1);
        }
    }

    void fill_signal_info()
    {
        info = new WFDB_Siginfo[number_of_signals];
        samples = new WFDB_Sample[number_of_signals];
        if(isigopen(const_cast<char*>(file_name.c_str()), info, number_of_signals)
            != number_of_signals)
        {
            exit(1);
        }
    }

    void fill_result_annotation()
    {
        result_annotation.subtyp = 0;
        result_annotation.chan = 0;
        result_annotation.num = 0;
        result_annotation.aux = NULL;
    }

    void set_samples_frequency()
    {
        if(sampfreq((char*)NULL) < 240. || sampfreq((char*)NULL) > 260.)
        {
            setifreq(250.);
        }
    }

    void get_annotation()
    {
        annotation_info.name = "qrs";
        annotation_info.stat = WFDB_WRITE;
        if(annopen(const_cast<char*>(file_name.c_str()), &annotation_info, 1) < 0)
        {
            exit(1);
        }
    }

    void getsc_min_max()
    {
        slope_criterion_min = muvadu(0, 1000);
        slope_criterion_max = 10 * slope_criterion_min;
    }

    void get_sample_intervals()
    {
        number_of_160ms_intervals = strtim("0.16");
        number_of_200ms_intervals = strtim("0.2");
        number_of_2s_intervals = strtim("2");
    }

    void init_samples()
    {
        getvec(samples);
        sample1 = sample2 = sample3 = sample4 = sample5 = sample6 = sample7 = sample8
            = sample9 = sample10 = samples[0];
    }

    void count_filter()
    {
        filter = (sample1 = samples[0]) + 4*sample2 + 6*sample3 + 4*sample4 + sample5
            -sample6 - 4*sample7 - 6*sample8 - 4*sample9 - sample10;
    }

    void adjust_threshold()
    {
        if(time % number_of_2s_intervals == 0)
        {
            if(slope_number == 0)
            {
                slope_criterion -= slope_criterion >> 4;
                if(slope_criterion < slope_criterion_min)
                {
                    slope_criterion = slope_criterion_min;
                }
            }
            else if(slope_number > 5)
            {
                slope_criterion += slope_criterion >> 4;
                if(slope_criterion > slope_criterion_max)
                {
                    slope_criterion = slope_criterion_max;
                }
            }
        }
    }

    void check_slope_found()
    {
        if(slope_number == 0 && abs(filter) > slope_criterion)
        {
            slope_number = 1;
            max_time = number_of_160ms_intervals;
            sign = (filter>0) ? 1:-1;
            qtime = time;
        }
    }

    void check_qrs_complex()
    {
        if(slope_number != 0)
        {
            if(filter*sign < -slope_criterion)
            {
                sign = -sign;
                max_time = (++slope_number > 4) ? number_of_200ms_intervals : number_of_160ms_intervals;
            }
            else if(filter * sign > slope_criterion && abs(filter) > maxslope)
            {
                maxslope = abs(filter);
            }
            if(max_time-- < 0)
            {
                if(slope_number >= 2 && slope_number <= 4)
                {
                    slope_criterion += ((maxslope>>2) - slope_criterion) >> 3;
                    if(slope_criterion < slope_criterion_min)
                    {
                        slope_criterion = slope_criterion_min;
                    }
                    else if(slope_scriterion > slope_criterion_max)
                    {
                        slope_criterion = slope_criterion_max;
                    }
                    result_annotation.time = strtim("i") - (time-qtime) -4;
                    result_annotation.anntyp = NORMAL;
                    (void)putann(0, &result_annotation);
                    time = 0;
                }
                else if(slope_number >=5)
                {
                    result_annotation.time = strtim("i") - (time-qtime) - 4;
                    result_annotation.anntyp = ARFCT;
                    (void)putann(0, &result_annotation);
                }
                slope_number = 0;
            }
        }
    }

    void update_samples()
    {
        sample10 = sample9;
        sample9 = sample8;
        sample8 = sample7;
        sample7 = sample6;
        sample6 = sample5;
        sample5 = sample4;
        sample4 = sample3;
        sample3 = sample2;
        sample2 = sample1;
    }

    void qrs_detection_algorithm()
    {
        do
        {
            count_filter();
            adjust_threshold();
            check_slope_found();
            check_qrs_complex();
            update_samples();
        }
        while(getvec(samples) > 0);
    }

private:
    std::string file_name;
    size_t number_of_signals;
    WFDB_Sample* samples;
    WFDB_Siginfo* info;
    WFDB_Anninfo annotation_info;
    WFDB_Annotation result_annotation;
    int slope_criterion_min;
    int slope_criterion_max;
    int slope_criterion;
    int slope_number;
    int number_of_160ms_intervals;
    int number_of_200ms_intervals;
    int number_of_2s_intervals;
    int max_time;
    int sample1, sample2, sample3, sample4, sample5, sample6,
        sample7, sample8, sample9, sample10;
    int filter;
    int time;
    int qtime;
    int sign;
    int maxslope;
};

int main()
{
    qrs_analyzer analyzer;
    analyzer.execute();
}
