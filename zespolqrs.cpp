#include <iostream>
#include <wfdb/wfdb.h>
#include <vector>

class qrs_analyzer
{
public:
    qrs_analyzer() : file_name(), number_of_signals(0), slope_criterion_min(0),
        annotation_info(), result_annotation(), slope_criterion_max(0), number_of_160ms_intervals(0),
        number_of_200ms_intervals(0), number_of_2s_intervals(0)
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
    }

    void execute()
    {
        set_samples_frequency();
        get_annotation();
        getsc_min_max();
        get_sample_intervals();
        init_samples();
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
        samples = new WFDB_Samples[number_of_signals];
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

private:
    std::string file_name;
    size_t number_of_signals;
    WFDB_Sample* samples;
    WFDB_Siginfo* info;
    WFDB_Anninfo annotation_info;
    WFDB_Ammotation result_annotation;
    int slope_criterion_min;
    int slope_criterion_max;
    int number_of_160ms_intervals;
    int number_of_200ms_intervals;
    int number_of_2s_intervals;
    int sample1, sample2, sample3, sample4, sample5, sample6,
        sample7, sample8, sample9, sample10;
};

int main()
{
    qrs_analyzer analyzer;
    analyzer.execute();
}
