#include "personbaggageview.h"
#include "ui_personbaggageview.h"

PersonBaggageView::PersonBaggageView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PersonBaggageView)
{
    ui->setupUi(this);
}

PersonBaggageView::~PersonBaggageView()
{
    delete ui;
}

void PersonBaggageView::clearViewData()
{
    // 目前详情页数据由外部喂入，先保留空实现，作为统一清理入口。
}


//视频回放按钮点击
void PersonBaggageView::on_videoBtn_clicked()
{

}

//上一张旅客照片
void PersonBaggageView::on_prevPassBtn_clicked()
{

}

//下一张旅客照片
void PersonBaggageView::on_nextPassBtn_clicked()
{

}

//上一张可见光行李照片
void PersonBaggageView::on_prevBagBtn_clicked()
{

}

//下一张可见光行李照片
void PersonBaggageView::on_nextBagBtn_clicked()
{

}

//返回
void PersonBaggageView::on_backButton_clicked()
{
    emit requestBack();
}

