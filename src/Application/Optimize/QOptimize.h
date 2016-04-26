#ifndef __QSHAPEWORKSOPTIMIZE_H__
#define __QSHAPEWORKSOPTIMIZE_H__

#include <ShapeWorksOptimize.h>
#include <ShapeWorksGroom.h>
#include <QObject>

class QOptimize : public QObject, public ShapeWorksOptimize {
Q_OBJECT;
public:
  QOptimize(QObject* parent = nullptr,
    std::vector<ImageType::Pointer> inputs =
    std::vector<ImageType::Pointer>(), size_t numScales = 1,
    std::vector<double> start_reg = std::vector<double>(),
    std::vector<double> end_reg = std::vector<double>(),
    std::vector<unsigned int> iters = std::vector<unsigned int>(),
    std::vector<double> tolerance = std::vector<double>(),
    std::vector<double> decay_span = std::vector<double>(),
    bool verbose = false);
protected:
  virtual void iterateCallback(
    itk::Object *caller, const itk::EventObject &);
signals:
  void progress(int);
};

#endif
