#ifndef __FS_LINE_LINK_H__
#define __FS_LINE_LINK_H__

// 连线信息
struct LineLink
{
    unsigned int startX = 0;
    unsigned int startY = 0;
    unsigned int startZ = 0;
    unsigned int startN = 0;

    unsigned int endX = 1;
    unsigned int endY = 0;
    unsigned int endZ = 0;
    unsigned int endN = 0;
};

#endif