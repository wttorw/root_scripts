#include <iostream>
#include <vector>
#include <cmath>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TMath.h>
#include <fstream>
#include <TLine.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TAxis.h>

using namespace std;



double intersection(double inf, double sup, TF1* f1, TF1* f2) {//  find the intersection between f1 and f2 in the range [inf,sup]
    auto finter = [f1, f2](double *x, double *par) {
        return TMath::Abs(f2->EvalPar(x, par) - f1->EvalPar(x, par));
    };
    TF1 fint("fint", finter, inf, sup, 0);
    return fint.GetMinimumX(inf,sup);
}



void post(TF1 &f,TGraphErrors *data){// calculate a posteriori error

	double postError=0.;	

	for(int i=0;i<data->GetN();i++){
	postError+=pow (f.Eval(data->GetX()[i])-data->GetY()[i],2);
	}
	postError=sqrt(postError/(data->GetN()-2));
	cout << "\nErrore a posteriori: " << postError << endl;
	for(int i=0;i<data->GetN();i++){
	data->SetPointError(i, 0,postError);
	}
}

Double_t fit(Double_t *x, Double_t *par) { //TF1 prototype
    Double_t X  = x[0];
    Double_t q = par[0];
    Double_t m = par[1];
    Double_t fitval = m*X + q;
    return fitval;
}


TGraphErrors* residui(TGraphErrors *gr, TF1 *f) {// calculate residuals
    auto *out = new TGraphErrors(); // Copy style properties from gr
    out->SetMarkerStyle(gr->GetMarkerStyle());
    out->SetMarkerColor(gr->GetMarkerColor());
    out->SetMarkerSize(gr->GetMarkerSize());
    
    out->SetName("residuals"); //Set a new name for clarit
    for (int i = 0; i < gr->GetN(); i++) {
        out->SetPoint(i, gr->GetPointX(i), f->Eval(gr->GetPointX(i)) - gr->GetPointY(i));
        out->SetPointError(i, gr->GetErrorX(i), gr->GetErrorY(i));
    }
    return out;
}

//plot two TF1 with residuals in the same canvas
void line2(const char *file1,const char *file2,float inf=0.,float sup=6000.){
    auto c1 = new TCanvas("c1", "c1", 200, 10, 800, 800); // Adjusted canvas size
    c1->SetFillColor(0);

    // Create two pads with different heights
    TPad *pad1 = new TPad("pad1", "pad1", 0.0, 0.3, 1.0, 1.0);
    TPad *pad2 = new TPad("pad2", "pad2", 0.0, 0.0, 1.0, 0.3);
    
    // Set grids for both pads
    pad1->SetGrid();
    pad2->SetGrid();

    // Draw pads on canvas
    pad1->Draw();
    pad2->Draw();

    auto y_name = "Energy [keV]";
    auto x_name = "Peak [a.u.]";

    // Create the first graph with errors
    auto *gr1 = new TGraphErrors(file1);
    auto *gr2 = new TGraphErrors(file2);
    
	
	
        
    auto size = 1.3;
    gr1->SetMarkerStyle(20);
    gr1->SetMarkerSize(size);
    gr1->SetMarkerColor(kBlue);
    gr2->SetMarkerStyle(22);
    gr2->SetMarkerSize(size);   
    gr2->SetMarkerColor(kRed);



    
    TF1 *ex1 = new TF1("ex1","pol1", inf, sup);
    ex1->SetParameters(0.,1.);
    ex1->SetParNames("q","m");
    TF1 *ex2 = new TF1("ex2", "pol1", inf, sup);
    ex2->SetParameters(0.,1.);
    ex2->SetParNames("q","m");


	
    ex1->SetLineColor(kBlue);
    ex2->SetLineColor(kRed);


    gr1->Fit("ex1","M");
    auto *res1 = residui(gr1,ex1);
    
    gr2->Fit("ex2","M");
    auto *res2 = residui(gr2,ex2);


    auto *grall = new TMultiGraph();
    grall->Add(gr1);
    grall->Add(gr1);
    grall->Add(gr2);

    grall->GetXaxis()->SetTitle(x_name);
    grall->GetYaxis()->SetTitle(y_name);
   

    // Draw the first graph on the first pad
    pad1->cd();
    grall->Draw("AP");
    //ex2->SetLineColor(kBlack);
    //ex2->Draw("same");

    ex1->SetLineColor(kBlack);
    auto legend = new TLegend(0.1203008,0.7013825,0.3358396,0.8894009,NULL,"brNDC");
    legend->AddEntry(ex1,"y = mx+q");
    legend->AddEntry(gr1,"Canale 0");//file1
    legend->AddEntry(gr2,"Canale 1");//file2
    legend->Draw();


    // Hide the X-axis on the first pad
    pad1->GetFrame()->SetX1(10);
    pad1->GetFrame()->SetX2(450);
    gr1->GetXaxis()->SetLabelOffset(999);
    gr1->GetXaxis()->SetTitleOffset(999);

    // Create the residuals graph
    auto *baseline = new TF1("baseline", "0", inf,sup);
    baseline->SetLineColor(kBlack);

    
    auto *resall = new TMultiGraph();
    resall->Add(res1);
    resall->Add(res2);
    // Set marker style for the residuals graph

    resall->GetXaxis()->SetTitle(x_name);
    resall->GetYaxis()->SetTitle("Residuals");
    
    resall->GetXaxis()->SetLabelSize(0.08); // Adjust the label size as needed
    resall->GetXaxis()->SetTitleSize(0.1); 
    resall->GetYaxis()->SetLabelSize(0.08);
    resall->GetYaxis()->SetTitleSize(0.1); 

    // Draw the residuals graph on the second pad
    pad2->cd();
    resall->Draw("AP");
    baseline->Draw("same");

    // Synchronize the X-axis of both pads
    pad1->SetBottomMargin(0);
    pad2->SetTopMargin(0);
    pad2->SetBottomMargin(0.3);

    // Update the canvas
    c1->Update();

    // Set style options for the fit
    gStyle->SetOptFit(10111);
}
