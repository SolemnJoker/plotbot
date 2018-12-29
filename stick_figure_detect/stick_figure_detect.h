#include<opencv2/core.hpp>
#include<memory>
#include<vector>
#include<string>
struct SPoint{
    int x;
    int y;
};

class StickCompress{
private:
    std::vector<int> box_;
public:
    cv::Point offset = {0,0};
    std::vector<SPoint> stick_point;  

    StickCompress();
    StickCompress(std::string file_name);
    
    void reset();
    void push_stick(int x, int y);
    void scale_to_size(int  width ,int height);
    std::string to_buf();
    void save(std::string file_name);
};

class StickFigure{
    class StickFigureImpl;
    std::unique_ptr<StickFigureImpl> impl_;
public:
    StickFigure();
    bool is_stick_figure(cv::Mat src, int threshold);
    cv::Mat to_binary_stick(int threshold);
private:
};

