/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2006 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Joerg Langenberg <joerg.langenberg@gmx.net>
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

//for matlab use oh this class import must be done different
#ifndef __DXF_MAP_H__
#include <main/dxf_map.h>
#endif // __DXF_MAP_H__

//
// Constructor and destructor
//

DxfMap::DxfMap(int maxFeatureNum)
{
    feature = (dxf_map_feature*)calloc(maxFeatureNum, sizeof(dxf_map_feature));

    if (feature != NULL)
    {
        this->maxFeatureNum = maxFeatureNum;
    }
    else
    {
        this->maxFeatureNum = 0;
    }

    featureNum = 0;

    xMin = 0.0;
    xMax = 0.0;
    yMin = 0.0;
    yMax = 0.0;
}

DxfMap::~DxfMap()
{
    if (feature != NULL)
    {
        free(feature);
    }
}

//
// dxf_map_read
//

int DxfMap::read_group(FILE *fp, char *string, int *number, double *real,
                       int *line)
{
    int    groupCode;
    char   buffer[256];

    string[0] = 0;
    *number   = 0;
    *real     = 0.0;

    (*line)++;
    if (fgets(buffer, 256, fp) == NULL)
    {
        printf("Can't read group code (line %i)\n", *line);
        return -EIO;
    }

    sscanf(buffer, "%i", &groupCode);

//    printf("read group code %i ", groupCode);

    (*line)++;
    if (fgets(buffer, 256, fp) == NULL)
    {
        printf("Can't read group data (line %i)\n", *line);
        return -EIO;
    }

    if (((groupCode >= 0) && (groupCode < 10)) || (groupCode == 999))
    {
        sscanf(buffer, "%s\n", string);
//        printf("string %s\n", string);
    }
    else if (((groupCode >= 10) && (groupCode < 60)) ||
             ((groupCode >= 210) && (groupCode < 240)))
    {
        sscanf(buffer, "%lf", real);
//        printf("real %f\n", *real);
    }
    else if ((groupCode >= 60) && (groupCode < 80))
    {
        sscanf(buffer, "%i\n", number);
//        printf("number %i\n", *number);
    }
    else
    {
        return 999;
//        printf("unknown groupCode %i\n", groupCode);
//        return -1;
    }
    return groupCode;
}

int DxfMap::read_polyline(FILE *fp)
{
    int    groupCode;
    char   string[256];
    int    number;
    double real;
    int    line;
    int    vertexNum = 0;
    double newX = 0.0;
    double newY = 0.0;
    double oldX = 0.0;
    double oldY = 0.0;

    oldX = 0.0;
    oldY = 0.0;
    newX = 0.0;
    newY = 0.0;

    while ((groupCode = read_group(fp, string, &number, &real, &line)) >= 0)
    {
        if (groupCode == 10)
        {
            newX = (double)real;
        }

        if (groupCode == 20)
        {
            newY = (double)real;
        }

        if ((groupCode == 0) && (strncmp(string, "VERTEX", 6) == 0))
        {
//            printf("VERTEX\n");

            if ((vertexNum >= 2) && (featureNum < maxFeatureNum))
            {
                // create new line
                feature[featureNum].x = oldX;
                feature[featureNum].y = oldY;
                feature[featureNum].x2 = newX;
                feature[featureNum].y2 = newY;
                featureNum++;
            }

            oldX = newX;
            oldY = newY;
            newX = 0.0;
            newY = 0.0;
            vertexNum++;
        }

        if ((groupCode == 0) && (strncmp(string, "SEQEND", 6) == 0))
        {
            if ((vertexNum >= 2) && (featureNum < maxFeatureNum))
            {
                // create new line
                feature[featureNum].x = oldX;
                feature[featureNum].y = oldY;
                feature[featureNum].x2 = newX;
                feature[featureNum].y2 = newY;
                featureNum++;
            }
            break;
        }
    }

    if (groupCode < 0)
    {
        return -EIO;
    }
    else
    {
        return vertexNum;
    }
}

int DxfMap::read_line(FILE *fp)
{
    int    groupCode;
    char   string[256];
    int    number;
    double real;
    int    line;
    int    x1, y1, x2, y2;

    x1 = 0;
    y1 = 0;
    x2 = 0;
    y2 = 0;

    if (featureNum < maxFeatureNum)
    {
        while ((groupCode = read_group(fp, string, &number, &real, &line)) >= 0)
        {
            if (groupCode == 10)
            {
                feature[featureNum].x = (double)real;
                x1 = 1;
            }

            if (groupCode == 20)
            {
                feature[featureNum].y = (double)real;
                y1 = 1;
            }

            if (groupCode == 11)
            {
                feature[featureNum].x2 = (double)real;
                x2 = 1;
            }

            if (groupCode == 21)
            {
                feature[featureNum].y2 = (double)real;
                y2 = 1;
            }

            if ((x1 == 1) && (y1 == 1) && (x2 == 1) && (y2 == 1))
                break;
        }
        featureNum++;
        return 1;
    }
    else
    {
        return -EIO;
    }
}

int DxfMap::load(char *filename, double mapOffsetX, double mapOffsetY)
{
    FILE   *fp;
    int    groupCode;
    char   string[256];
    int    number;
    double real;
    int    line;
    int    i;
    double x, y;

    if ((fp = fopen(filename, "r")) == NULL)
    {
        printf("Can't open dxf file \"%s\"\n", filename);
        featureNum = 0;
        return -EIO;
    }

    line = 0;

    while ((groupCode = read_group(fp, string, &number, &real, &line)) >= 0)
    {
        if ((groupCode == 0) && (strncmp(string, "POLYLINE", 9) == 0))
        {
//            printf("POLYLINE\n");
            if (read_polyline(fp) < 0)
            {
                break;
            }
        }

        if ((groupCode == 0) && (strncmp(string, "LINE", 5) == 0))
        {
//            printf("LINE\n");
            if (read_line(fp) < 0)
            {
                break;
            }
        }

        if ((groupCode == 0) && (strncmp(string, "EOF", 3) == 0))
            break;
    }

    fclose(fp);

    for (i = 0; i < featureNum; i++)
    {
        x = (feature[i].y - mapOffsetX) * 1000.0;
        y = (feature[i].x - mapOffsetY) * 1000.0;
        feature[i].x = x;
        feature[i].y = y;

        x = (feature[i].y2 - mapOffsetX) * 1000.0;
        y = (feature[i].x2 - mapOffsetY) * 1000.0;
        feature[i].x2 = x;
        feature[i].y2 = y;

        x = feature[i].x2 - feature[i].x;
        y = feature[i].y2 - feature[i].y;

        feature[i].l   = sqrt(x * x + y * y);

        if (feature[i].l > 0.0)
        {
            feature[i].phi = atan2(y, x);
            feature[i].sin = sin(feature[i].phi);
            feature[i].cos = cos(feature[i].phi);
        }
        else
        {
            feature[i].phi = 0.0;
            feature[i].sin = 0.0;
            feature[i].cos = 1.0;
        }
    }

    return 0;
}

//
// dxf_map_write
//

int DxfMap::write_string(FILE *fp, int groupCode, char *string)
{
    int ret;

    ret = fprintf(fp, "%i\n", groupCode);
    if (ret < 0)
    {
        printf("Can't write groupCode (%i)\n", ret);
        return ret;
    }

    ret = fprintf(fp, "%s\n", string);
    if (ret < 0)
    {
        printf("Can't write string (%i)\n", ret);
        return ret;
    }

    return 0;
}

int DxfMap::write_real(FILE *fp, int groupCode, double real)
{
    int ret;

    ret = fprintf(fp, "%i\n", groupCode);
    if (ret < 0)
    {
        printf("Can't write groupCode (%i)\n", ret);
        return ret;
    }

    ret = fprintf(fp, "%f\n", real);
    if (ret < 0)
    {
        printf("Can't write real (%i)\n", ret);
        return ret;
    }

    return 0;
}

int DxfMap::write_number(FILE *fp, int groupCode, int number)
{
    int ret;

    ret = fprintf(fp, "%i\n", groupCode);
    if (ret < 0)
    {
        printf("Can't write groupCode (%i)\n", ret);
        return ret;
    }

    ret = fprintf(fp, "%i\n", number);
    if (ret < 0)
    {
        printf("Can't write number (%i)\n", ret);
        return ret;
    }

    return 0;
}

void DxfMap::calcBounds()
{
    int i;

    xMin = feature[0].x;
    yMin = feature[0].y;
    xMax = feature[0].x;
    yMax = feature[0].y;

    for (i = 0; i < featureNum; i++)
    {
        if (feature[i].x < xMin)
        {
            xMin = feature[i].x;
        }
        if (feature[i].y < yMin)
        {
            yMin = feature[i].y;
        }
        if (feature[i].x > xMax)
        {
            xMax = feature[i].x;
        }
        if (feature[i].y > yMax)
        {
            yMax = feature[i].y;
        }

        if (feature[i].x2 < xMin)
        {
            xMin = feature[i].x2;
        }
        if (feature[i].y2 < yMin)
        {
            yMin = feature[i].y2;
        }
        if (feature[i].x2 > xMax)
        {
            xMax = feature[i].x2;
        }
        if (feature[i].y2 > yMax)
        {
            yMax = feature[i].y2;
        }
    }
}

int DxfMap::write_head(FILE *fp)
{
    calcBounds();

    // head
    write_string(fp, 999, "DXF Map, RTS, University of Hannover");
    write_string(fp, 0, "SECTION");
    write_string(fp, 2, "HEADER");
    write_string(fp, 9, "$ACADVER");
    write_string(fp, 1, "AC1006");
    write_string(fp, 9, "$INSBASE");
    write_real  (fp, 10, 0.0);
    write_real  (fp, 20, 0.0);
    write_real  (fp, 30, 0.0);
    write_string(fp, 9, "$EXTMIN");
    write_real  (fp, 10, xMin);
    write_real  (fp, 20, yMin);
    write_real  (fp, 30, 0.0);
    write_string(fp, 9, "$EXTMAX");
    write_real  (fp, 10, xMax);
    write_real  (fp, 20, yMax);
    write_real  (fp, 30, 0.0);
    write_string(fp, 0, "ENDSEC");
    // Tables
    write_string(fp, 0, "SECTION");
    write_string(fp, 2, "TABLES");
    write_string(fp, 0, "TABLE");
    write_string(fp, 2, "LTYPE");
    write_number(fp, 70, 1);
    write_string(fp, 0, "LTYPE");
    write_string(fp, 2, "CONTINUOUS");
    write_number(fp, 70, 64);
    write_string(fp, 3, "Solid line");
    write_number(fp, 72, 65);
    write_number(fp, 73, 0);
    write_real  (fp, 40, 0.0);
    write_string(fp, 0, "ENDTAB");
    write_string(fp, 0, "TABLE");
    write_string(fp, 2, "LAYER");
    write_number(fp, 70, 6);
    write_string(fp, 0, "LAYER");
    write_number(fp, 2, 1);
    write_number(fp, 70, 64);
    write_number(fp, 62, 7);
    write_string(fp, 6, "CONTINUOUS");
    write_string(fp, 0, "LAYER");
    write_number(fp, 2, 2);
    write_number(fp, 70, 64);
    write_number(fp, 62, 7);
    write_string(fp, 6, "CONTINUOUS");
    write_string(fp, 0, "ENDTAB");
    write_string(fp, 0, "TABLE");
    write_string(fp, 2, "STYLE");
    write_number(fp, 70, 0);
    write_string(fp, 0, "ENDTAB");
    write_string(fp, 0, "ENDSEC");
    // empty block section
    write_string(fp, 0, "SECTION");
    write_string(fp, 2, "BLOCKS");
    write_string(fp, 0, "ENDSEC");
    // start Entities
    write_string(fp, 0, "SECTION");
    write_string(fp, 2, "ENTITIES");

    return 0;
}

int DxfMap::write_eof(FILE *fp)
{
    // end Entities
    write_string(fp, 0, "ENDSEC");
    // EOF
    write_string(fp, 0, "EOF");

    return 0;
}

int DxfMap::write_line(FILE *fp, dxf_map_feature *feature)
{
    write_string(fp, 0, "LINE");
    write_number(fp, 8, 0);       // layer
    write_number(fp, 62, 0);      // color
    write_real  (fp, 10, feature->x);
    write_real  (fp, 20, feature->y);
    write_real  (fp, 30, 0.0);
    write_real  (fp, 11, feature->x2);
    write_real  (fp, 21, feature->y2);
    write_real  (fp, 31, 0.0);

    return 0;
}

int DxfMap::save(char *filename)
{
    FILE   *fp;
    int    i;
    double x, y;

    for (i = 0; i < featureNum; i++)
    {
        x = feature[i].y / 1000.0;
        y = feature[i].x / 1000.0;
        feature[i].x = x;
        feature[i].y = y;

        x = feature[i].y2 / 1000.0;
        y = feature[i].x2 / 1000.0;
        feature[i].x2 = x;
        feature[i].y2 = y;
    }

    if ((fp = fopen(filename, "w")) == NULL)
    {
        printf("Can't open dxf file \"%s\"\n", filename);
        return -EIO;
    }

    write_head(fp);

    for (i = 0; i < featureNum; i ++)
    {
        write_line(fp, &feature[i]);
    }

    write_eof(fp);

    fclose(fp);
    return 0;
}
