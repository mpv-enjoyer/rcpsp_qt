// mymodel.h
#ifndef RESULTMODEL_H
#define RESULTMODEL_H

#include <QAbstractTableModel>
#include <QModelIndex>
#include <QDebug>
#include <QPixmap>
#include "algorithm.h"

class ResultModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit ResultModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    void append(ResultPair value);
    void update(int idx, ResultPair value);
    void deleteRow(int idx);
    void insertAt(int idx, ResultPair value);
    void populate(QList<ResultPair> *newValues);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
private:
    QList<ResultPair>* values;
};

#endif
