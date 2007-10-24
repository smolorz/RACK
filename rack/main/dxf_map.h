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
#ifndef __DXF_MAP_H__
#define __DXF_MAP_H__

typedef struct {
    double x;
    double y;
    double x2;
    double y2;
    double l;
    double rho;
    double sin;
    double cos;
} dxf_map_feature;

//######################################################################
//# class DxfMap
//######################################################################

class DxfMap
{
    private:

        int maxFeatureNum;

    protected:

        int read_group(FILE *fp, char *string, int *number, double *real, int *line);
        int read_polyline(FILE *fp);
        int read_line(FILE *fp);

        int write_string(FILE *fp, int groupCode, char *string);
        int write_real(FILE *fp, int groupCode, double real);
        int write_number(FILE *fp, int groupCode, int number);
        int write_head(FILE *fp);
        int write_eof(FILE *fp);
        int write_line(FILE *fp, dxf_map_feature *feature);

        void calcBounds();

    public:

        DxfMap(int maxFeatureNum);
        ~DxfMap();

        dxf_map_feature *feature;
        int featureNum;

        double xMin;
        double xMax;
        double yMin;
        double yMax;

        int save(char *filename);
        int load(char *filename, double mapOffsetX, double mapOffsetY, double scaleFactor);
        int load(char *filename, double mapOffsetX, double mapOffsetY)
        {
            return load(filename, mapOffsetX, mapOffsetY, 1000.0);
        }
        int load(char *filename)
        {
            return load(filename, 0.0, 0.0, 1000.0);
        }
};

#endif // __DXF_MAP_H__
