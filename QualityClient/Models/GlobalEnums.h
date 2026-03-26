#ifndef GLOBALENUMS_H
#define GLOBALENUMS_H


enum ServerType
{
    Unknown = -1,
    YiSuo = 700,
    TongFang = 701,
    HaiMan = 699
};

enum ReExtractStatus
{
    /// <summary>
    /// 未匹配（对应 null）
    /// </summary>
    Unmatched = -1,

    /// <summary>
    /// 无需重抽
    /// </summary>
    NoNeed = 0,

    /// <summary>
    /// 可重抽
    /// </summary>
    CanReExtract = 1,

    /// <summary>
    /// 已添加重抽任务
    /// </summary>
    TaskAdded = 2,

    /// <summary>
    /// 重抽超时
    /// </summary>
    Timeout = 3,

    /// <summary>
    /// 重抽完成
    /// </summary>
    Completed = 4
};

enum QualityImageResult
{
    None = 0,
    EndCheck = 1,
    StartCheck = 2
};


#endif // GLOBALENUMS_H
