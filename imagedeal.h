#ifndef IMAGEDEAL_H
#define IMAGEDEAL_H

#include "global.h"
#include "excv.h"


using namespace HalconCpp;
namespace MSerials {


//找到多个检测点
static void draw_retangle_mult (HObject *ho_ROI, HTuple hv_WindowHandle, HTuple hv_Num,
    HTuple hv_RowIn, HTuple hv_ColumnIn, HTuple hv_AngleIn, HTuple hv_Len1In, HTuple hv_Len2In,
    HTuple *hv_Row, HTuple *hv_Column, HTuple *hv_Len2)
{

  // Local iconic variables
  HObject  ho_ROITmp;

  // Local control variables
  HTuple  hv_Len2Origin, hv_Len2Div, hv_RStart;
  HTuple  hv_CStart, hv_i, hv_RowTmp, hv_ColTmp;

  hv_Len2Origin = hv_Len2In/hv_Num;
  hv_Len2Div = (2*hv_Len2In)/hv_Num;
  //计算除执行的位置
  GenEmptyRegion(&(*ho_ROI));
  hv_RStart = (hv_RowIn-(hv_Len2In*(hv_AngleIn.TupleCos())))+((hv_Len2Div/2)*(hv_AngleIn.TupleCos()));
  hv_CStart = (hv_ColumnIn-(hv_Len2In*(hv_AngleIn.TupleSin())))+((hv_Len2Div/2)*(hv_AngleIn.TupleSin()));
  (*hv_Row) = HTuple();
  (*hv_Column) = HTuple();
  (*hv_Len2) = hv_Len2Div/2;
  {
  HTuple end_val9 = hv_Num-1;
  HTuple step_val9 = 1;
  for (hv_i=0; hv_i.Continue(end_val9, step_val9); hv_i += step_val9)
  {
    hv_RowTmp = hv_RStart+((hv_i*hv_Len2Div)*(hv_AngleIn.TupleCos()));
    hv_ColTmp = hv_CStart+((hv_i*hv_Len2Div)*(hv_AngleIn.TupleSin()));
    GenRectangle2(&ho_ROITmp, hv_RowTmp, hv_ColTmp, hv_AngleIn, hv_Len1In, hv_Len2Div/2);

    DispObj(ho_ROITmp,hv_WindowHandle);
    (*hv_Row)[hv_i] = hv_RowTmp;
    (*hv_Column)[hv_i] = hv_ColTmp;
    Union2((*ho_ROI), ho_ROITmp, &(*ho_ROI));
  }
  }
  return;
}

static PositionInfo GetAngleAndLine(HObject &ho_MeasureImage,HTuple WindowHandle,HTuple hv_RowIn, HTuple hv_ColumnIn, HTuple hv_AngleIn, HTuple hv_Len1In, HTuple hv_Len2In, int sample = 5, double thresVal = -1, double offset = 0)
{


    HTuple hv_Num = sample;
    HObject  ho_ROI, ho_Cross, ho_Contour, ho_Crosses;

    HTuple hv_Width=0, hv_Height=0;
    HTuple  hv_Row = 0, hv_Column = 0;
    HTuple  hv_Len2 =5, hv_RowLen, hv_RowLine, hv_ColLine, hv_j,hv_Angle_X;
    HTuple  hv_MeasureHandle, hv_RowEdgeFirst, hv_ColumnEdgeFirst;
    HTuple  hv_AmplitudeFirst, hv_IntraDistance, hv_except;
    HTuple  hv_RowBegin1, hv_ColBegin1, hv_RowEnd1, hv_ColEnd1;
    HTuple  hv_Nr1, hv_Nc1, hv_Dist1;

    try{
        if (HDevWindowStack::IsOpen())
          SetDraw(HDevWindowStack::GetActive(),"margin");
        if (HDevWindowStack::IsOpen())
          SetColor(HDevWindowStack::GetActive(),"cyan");
        PositionInfo Pos;
        GetImageSize(ho_MeasureImage, &hv_Width, &hv_Height);
        GenEmptyObj(&ho_ROI);
        draw_retangle_mult(&ho_ROI,  WindowHandle, hv_Num, hv_RowIn, hv_ColumnIn, hv_AngleIn,
            hv_Len1In, hv_Len2In, &hv_Row, &hv_Column, &hv_Len2);
        if (HDevWindowStack::IsOpen())
          DispObj(ho_ROI, HDevWindowStack::GetActive());
        hv_RowLen = hv_Row.TupleLength();
        hv_RowLine = HTuple();
        hv_ColLine = HTuple();
        {
        HTuple end_val49 = hv_RowLen-1;
        HTuple step_val49 = 1;
        for (hv_j=0; hv_j.Continue(end_val49, step_val49); hv_j += step_val49)
        {
          GenMeasureRectangle2(HTuple(hv_Row[hv_j]), HTuple(hv_Column[hv_j]), hv_AngleIn,
              hv_Len1In, hv_Len2, hv_Width, hv_Height, "bilinear", &hv_MeasureHandle);
          hv_RowEdgeFirst = HTuple();
          hv_ColumnEdgeFirst = HTuple();
          try
          {
            MeasurePos(ho_MeasureImage, hv_MeasureHandle, 1.0, 20.0, "positive", "first",
                &hv_RowEdgeFirst, &hv_ColumnEdgeFirst, &hv_AmplitudeFirst, &hv_IntraDistance);
            GenCrossContourXld(&ho_Cross, HTuple(hv_RowEdgeFirst[0]), HTuple(hv_ColumnEdgeFirst[0]),
                20, 0.785398);
            if (HDevWindowStack::IsOpen())
              DispObj(ho_Cross, HDevWindowStack::GetActive());
            hv_RowLine = hv_RowLine.TupleConcat(HTuple(hv_RowEdgeFirst[0]));
            hv_ColLine = hv_ColLine.TupleConcat(HTuple(hv_ColumnEdgeFirst[0]));
          }
          // catch (except)
          catch (HException &HDevExpDefaultException)
          {
            HDevExpDefaultException.ToHTuple(&hv_except);
          }
        }
        }
        if (0 != ((hv_RowLine.TupleLength())<2))
        {
        }
        GenEmptyObj(&ho_Contour);
        GenEmptyObj(&ho_Crosses);
        GenCrossContourXld(&ho_Crosses, hv_RowLine, hv_ColLine, 20, 0.785398);
        GenContourPolygonXld(&ho_Contour, hv_RowLine, hv_ColLine);
        FitLineContourXld(ho_Contour, "regression", -1, 0, 5, 1.345, &hv_RowBegin1, &hv_ColBegin1,
            &hv_RowEnd1, &hv_ColEnd1, &hv_Nr1, &hv_Nc1, &hv_Dist1);
        if (HDevWindowStack::IsOpen())
          SetColor(HDevWindowStack::GetActive(),"cyan");
        if (HDevWindowStack::IsOpen())
          DispObj(ho_Contour, HDevWindowStack::GetActive());
        if (HDevWindowStack::IsOpen())
          SetColor(HDevWindowStack::GetActive(),"#0000AF");
        DispLine(HDevWindowStack::GetActive(), hv_RowBegin1, hv_ColBegin1, hv_RowEnd1, hv_ColEnd1);
        AngleLx(hv_RowBegin1, hv_ColBegin1, hv_RowEnd1, hv_ColEnd1, &hv_Angle_X);

        Pos.Angles.at(0) = hv_Angle_X[0].D();

        if(abs(offset) > 0){
         //   Pos.Points.at(0).x = offset*sin(Pos.Angles.at(0)*180/3.1415926) + hv_ColBegin1[0].D();
         //   Pos.Points.at(0).y = offset*cos(Pos.Angles.at(0)*180/3.1415926) + hv_RowBegin1[0].D();
         //   Pos.Points.at(1).x = offset*sin(Pos.Angles.at(0)*180/3.1415926) + hv_ColEnd1[0].D();
         //   Pos.Points.at(1).y = offset*cos(Pos.Angles.at(0)*180/3.1415926) + hv_RowEnd1[0].D();

            Pos.Points.at(0).x = offset*sin(Pos.Angles.at(0)*1) + hv_ColBegin1[0].D();
            Pos.Points.at(0).y = offset*cos(Pos.Angles.at(0)*1) + hv_RowBegin1[0].D();
            Pos.Points.at(1).x = offset*sin(Pos.Angles.at(0)*1) + hv_ColEnd1[0].D();
            Pos.Points.at(1).y = offset*cos(Pos.Angles.at(0)*1) + hv_RowEnd1[0].D();
            SetColor(HDevWindowStack::GetActive(),"#AF00AF");
            DispLine(HDevWindowStack::GetActive(), Pos.Points.at(0).y, Pos.Points.at(0).x, Pos.Points.at(1).y, Pos.Points.at(1).x);
        }else
        {
            Pos.Points.at(0).x = hv_ColBegin1[0].D();
            Pos.Points.at(0).y = hv_RowBegin1[0].D();
            Pos.Points.at(1).x = hv_ColEnd1[0].D();
            Pos.Points.at(1).y = hv_RowEnd1[0].D();
        }

        return Pos;
    }catch(HalconCpp::HException ex)
    {
        throw ex;
    }catch(std::out_of_range ex){
        throw std::move(ex);
    }catch(cv::Exception ex){
        throw ex;
    }
}



static int FindWhichClass(double Value)
{
    int Class = -1;
    double Base = SIZEPARA[0];
    if(Value < Base){
        return 0;
    }
    double Upper = Base;
    for(int i = 0; i < (MAX_SIZE_OF_DI-1);i++){
        Upper = Base + SIZEPARA[i+1];
        if(Value >= Base && Value<Upper){
            Class = i+1;
            if(Class >= USER_MAX_SIZE_OF_ID){
                Class = USER_MAX_SIZE_OF_ID;
                return Class;
            }

            return Class;
        }
        Base = Upper;
    }

    if(Class >= USER_MAX_SIZE_OF_ID){
        Class = USER_MAX_SIZE_OF_ID;
        return Class;
    }

    if(Class < 0)
        Class = 0;
    return Class;
    std::cout <<"class is "<<Class<<std::endl;
    return MAX_SIZE_OF_DI;
}


static PositionInfo GetResult(HObject &ho_MeasureImage, HTuple WindowHandle, double Row[] ,double Col[], double Angle[], double Len1[],double Len2[], int _SAMPLENUM = 5, double Threshold_Var = -1.0, double L1Offset = 0, double L2Offset = 0, double L3Offset = 0, double L4Offset = 0){
    // Local iconic variables

    try{

        PositionInfo Pos;

        HObject  ho_ROI_0, ho_TMP_Region, ho_Red;
        HObject  ho_Green, ho_Blue, ho_Hue, ho_Saturation, ho_Intensity;

        // Local control variables
        HTuple  hv_AcqHandle, hv_MeanH, hv_DeviationH;
        HTuple  hv_MeanS, hv_DeviationS, hv_MeanI, hv_DeviationI;
        Decompose3(ho_MeasureImage, &ho_Red, &ho_Green, &ho_Blue);
        TransFromRgb(ho_Red, ho_Green, ho_Blue, &ho_Hue, &ho_Saturation, &ho_Intensity,
            "hsv");
        GenRectangle2(&ho_ROI_0, Row[0], Col[0], Angle[0], Len1[0], Len2[0]);
        Intensity(ho_ROI_0, ho_Hue, &hv_MeanH, &hv_DeviationH);
        Intensity(ho_ROI_0, ho_Saturation, &hv_MeanS, &hv_DeviationS);
        Intensity(ho_ROI_0, ho_Intensity, &hv_MeanI, &hv_DeviationI);
        
        Pos.H = hv_MeanH[0].D();
        Pos.S = hv_MeanS[0].D();
        Pos.V = hv_MeanI[0].D();
        Pos.gama = hv_DeviationI[0].D();
        return Pos;
        
        
        HObject ho_Circle;
        HTuple hv_serital_str,hv_Row,hv_Col,hv_IsOverLap,hv_str_point1,hv_str_point2,hv_str_point3,hv_str_point4 ;
        Excv::h_disp_obj(ho_MeasureImage, WindowHandle);

        PositionInfo Pos1 =  GetAngleAndLine(ho_MeasureImage,WindowHandle,Row[0],Col[0],Angle[0],Len1[0],Len2[0],_SAMPLENUM,Threshold_Var,L1Offset);
        PositionInfo Pos2 =  GetAngleAndLine(ho_MeasureImage,WindowHandle,Row[1],Col[1],Angle[1],Len1[1],Len2[1],_SAMPLENUM,Threshold_Var,L2Offset);
        PositionInfo Pos3 =  GetAngleAndLine(ho_MeasureImage,WindowHandle,Row[2],Col[2],Angle[2],Len1[2],Len2[2],_SAMPLENUM,Threshold_Var,L3Offset);
        PositionInfo Pos4 =  GetAngleAndLine(ho_MeasureImage,WindowHandle,Row[3],Col[3],Angle[3],Len1[3],Len2[3],_SAMPLENUM,Threshold_Var,L4Offset);
        if (HDevWindowStack::IsOpen())
          SetColor(HDevWindowStack::GetActive(),"green");
        IntersectionLines(Pos1.Points.at(0).y, Pos1.Points.at(0).x, Pos1.Points.at(1).y, Pos1.Points.at(1).x,
            Pos2.Points.at(0).y, Pos2.Points.at(0).x, Pos2.Points.at(1).y, Pos2.Points.at(1).x, &hv_Row, &hv_Col, &hv_IsOverLap);

        GenEmptyObj(&ho_Circle);
        GenCircleContourXld(&ho_Circle, hv_Row, hv_Col, 20, 0, 6.28, "positive", 2.0);
        SetTposition(HDevWindowStack::GetActive(), hv_Row, hv_Col);
        hv_Col = hv_Col*RATIOX;
        hv_Row = hv_Row*RATIOY;
        hv_str_point1 = (("x:"+(hv_Col.TupleString(".5")))+"y:")+(hv_Row.TupleString(".5"));
        WriteString(HDevWindowStack::GetActive(), "点1 "+hv_str_point1);
        Pos.Points.at(0).x = hv_Col[0].D();
        Pos.Points.at(0).y = hv_Row[0].D();

        if (HDevWindowStack::IsOpen())
          DispObj(ho_Circle, HDevWindowStack::GetActive());

        IntersectionLines(Pos3.Points.at(0).y, Pos3.Points.at(0).x, Pos3.Points.at(1).y, Pos3.Points.at(1).x,
            Pos2.Points.at(0).y, Pos2.Points.at(0).x, Pos2.Points.at(1).y, Pos2.Points.at(1).x, &hv_Row, &hv_Col, &hv_IsOverLap);

        GenEmptyObj(&ho_Circle);
        GenCircleContourXld(&ho_Circle, hv_Row, hv_Col, 20, 0, 6.28, "positive", 2.0);
        SetTposition(HDevWindowStack::GetActive(), hv_Row, hv_Col);
        hv_Col = hv_Col*RATIOX;
        hv_Row = hv_Row*RATIOY;
        hv_str_point2 = (("x:"+(hv_Col.TupleString(".5")))+"y:")+(hv_Row.TupleString(".5"));
        WriteString(HDevWindowStack::GetActive(), "点2 "+hv_str_point2);
        Pos.Points.at(1).x = hv_Col[0].D();
        Pos.Points.at(1).y = hv_Row[0].D();

        if (HDevWindowStack::IsOpen())
          DispObj(ho_Circle, HDevWindowStack::GetActive());

        IntersectionLines(Pos3.Points.at(0).y, Pos3.Points.at(0).x, Pos3.Points.at(1).y, Pos3.Points.at(1).x,
            Pos4.Points.at(0).y, Pos4.Points.at(0).x, Pos4.Points.at(1).y, Pos4.Points.at(1).x, &hv_Row, &hv_Col, &hv_IsOverLap);

        GenEmptyObj(&ho_Circle);
        GenCircleContourXld(&ho_Circle, hv_Row, hv_Col, 20, 0, 6.28, "positive", 2.0);
        SetTposition(HDevWindowStack::GetActive(), hv_Row, hv_Col);
        hv_Col = hv_Col*RATIOX;
        hv_Row = hv_Row*RATIOY;
        hv_str_point3 = (("x:"+(hv_Col.TupleString(".5")))+"y:")+(hv_Row.TupleString(".5"));
        WriteString(HDevWindowStack::GetActive(), "点3 "+hv_str_point3);
        Pos.Points.at(2).x = hv_Col[0].D();
        Pos.Points.at(2).y = hv_Row[0].D();

        if (HDevWindowStack::IsOpen())
          DispObj(ho_Circle, HDevWindowStack::GetActive());

        IntersectionLines(Pos1.Points.at(0).y, Pos1.Points.at(0).x, Pos1.Points.at(1).y, Pos1.Points.at(1).x,
            Pos4.Points.at(0).y, Pos4.Points.at(0).x, Pos4.Points.at(1).y, Pos4.Points.at(1).x, &hv_Row, &hv_Col, &hv_IsOverLap);

        GenEmptyObj(&ho_Circle);
        GenCircleContourXld(&ho_Circle, hv_Row, hv_Col, 20, 0, 6.28, "positive", 2.0);
        SetTposition(HDevWindowStack::GetActive(), hv_Row, hv_Col);
        hv_Col = hv_Col*RATIOX;
        hv_Row = hv_Row*RATIOY;
        hv_str_point4 = (("x:"+(hv_Col.TupleString(".5")))+"y:")+(hv_Row.TupleString(".5"));
        WriteString(HDevWindowStack::GetActive(), "点4 "+hv_str_point4);
        Pos.Points.at(3).x = hv_Col[0].D();
        Pos.Points.at(3).y = hv_Row[0].D();

        if (HDevWindowStack::IsOpen())
          DispObj(ho_Circle, HDevWindowStack::GetActive());
        HTuple Angle_X = Pos1.Angles.at(0);
        Pos.Angles.at(0) = Pos1.Angles.at(0);

        hv_serital_str = (((hv_str_point1+hv_str_point2)+hv_str_point3)+hv_str_point4) + (("A:" + Angle_X.TupleString(".5")) + "\r\n");
        Pos.SerialsInfo = std::string(hv_serital_str[0].C());
        return Pos;

        }catch(HalconCpp::HException ex)
    {
        throw ex;
    }catch(std::out_of_range ex){
        throw std::exception((std::string("OutOfRange In CheckColor ")+ex.what()).c_str());
        throw std::move(ex);
    }catch(cv::Exception ex){
        throw std::exception((std::string("Opencv Error In CheckColor ")+ex.what()).c_str());
        throw ex;
    }
    }//EndGet
}//EndMserials

#endif // IMAGEDEAL_H
