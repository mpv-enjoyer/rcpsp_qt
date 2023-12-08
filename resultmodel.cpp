#include "resultmodel.h"

ResultModel::ResultModel(QObject *parent)
    :QAbstractListModel(parent)
{
    values = new QList<ResultPair>();
}

int ResultModel::rowCount(const QModelIndex &) const
{
    return values->count();
}

int ResultModel::columnCount(const QModelIndex &) const
{
    return 4;
}

QVariant ResultModel::data( const QModelIndex &index, int role ) const
{

    QVariant value;

    switch ( role )
    {
    case Qt::DisplayRole: //string
    {
        switch (index.column()) {
        case 0: {
            value = this->values->at(index.row()).job_id;
            break;
        }
        case 1: {
            value = this->values->at(index.row()).worker_group_id;
            break;
        }
        case 2: {
            value = this->values->at(index.row()).worker_internal_id;
            break;
        }
        case 3: {
            value = this->values->at(index.row()).start;
            break;
        }
        }
    }
    break;

    case Qt::UserRole: //data
    {
        value = this->values->at(index.row()).job_id;
    }
    break;

    default:
        break;
    }

    return value;
}

void ResultModel::populate(QList<ResultPair> *newValues)
{
    int idx = this->values->count();
    this->beginInsertRows(QModelIndex(), 1, idx);
    this->values = newValues;
    endInsertRows();
}

void ResultModel::append(ResultPair value)
{
    int newRow = this->values->count()+1;

    this->beginInsertRows(QModelIndex(), newRow, newRow);
    values->append(value);
    endInsertRows();
}

void ResultModel::update(int idx, ResultPair value)
{
    (*this->values)[idx] = value;

    QModelIndex item_idx_s = this->index(idx,0);
    QModelIndex item_idx_e = this->index(idx,this->columnCount(QModelIndex()));

    emit this->dataChanged(item_idx_s ,item_idx_e );
}

void ResultModel::deleteRow(int idx)
{
    this->beginRemoveRows(QModelIndex(), idx,idx);

    (*this->values).removeAt(idx);

    this->endRemoveRows();
}

void ResultModel::insertAt(int idx, ResultPair value)
{

    int newRow = idx;

    this->beginInsertRows(QModelIndex(), newRow, newRow);

    values->insert(newRow,value);

    endInsertRows();
}

QVariant ResultModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
            return QString("Job ID");
        case 1:
            return QString("Worker group ID");
        case 2:
            return QString("Worker internal ID");
        case 3:
            return QString("Begin at");
        }
    }
    return QVariant();
}
