#include <iostream>
#include <vector>
#include <cmath>
#include <TF1.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TMath.h>
#include <fstream>
#include <TLine.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TAxis.h>

using namespace std;


Double_t fit(Double_t *x, Double_t *par) {
    Double_t f  = x[0];
    Double_t tau = par[0];
    Double_t off = par[1];

    Double_t fitval = 20*log10(f*tau/(tau*tau+f*f))+off;
    return fitval;
}

Double_t fitd(Double_t *x, Double_t *par) {//fit for 2 different ft
    Double_t f  = x[0];
    Double_t a = par[0];
    Double_t b = par[1];

    Double_t fitval = 20*log10(f*b/sqrt(pow(a*b-f*f,2)+f*f*pow(a+b,2)));
    return fitval;
}

TGraphErrors* residui(TGraphErrors *gr, TF1 *f) {//create residuals from gr
    auto *out = new TGraphErrors(); // Copy properties from gr (gr->Clone() can couse problems in this context)
    out->SetMarkerStyle(gr->GetMarkerStyle());
    out->SetMarkerColor(gr->GetMarkerColor());
    out->SetName("residuals"); // Set Name for Clarity
    for (int i = 0; i < gr->GetN(); i++) {
        out->SetPoint(i, gr->GetPointX(i),- f->Eval(gr->GetPointX(i)) + gr->GetPointY(i));
        out->SetPointError(i, gr->GetErrorX(i), gr->GetErrorY(i));
    }
    return out;
}

void tau(float ft, float sft){//calculate tau (time costant)
	float t=1/(2*M_PI*ft);
	float st=1/(2*M_PI*ft*ft)*sft;
	cout << "tau: "<< t << "+/-" << st << endl;
}


void spice(TGraphErrors &gr){//conversion from LTSpice data
	    for(int i=0;i<gr.GetN();i++){
    	gr.SetPointX(i,gr.GetPointX(i)*0.001);
    	gr.SetPointError(i,0.,0.);
    	}
    }




void bode_form() {
    auto c1 = new TCanvas("c1", "c1", 200, 10, 800, 800); // Adjusted canvas size
    c1->SetFillColor(0);
    c1->SetGrid();

    // Create two pads with different heights
    TPad *pad1 = new TPad("pad1", "pad1", 0.0, 0.3, 1.0, 1.0);
    TPad *pad2 = new TPad("pad2", "pad2", 0.0, 0.0, 1.0, 0.3);
        



    // Draw pads on canvas
    pad1->Draw();
    pad2->Draw();

    

    // Create the first graph with errors
    auto *gr1 = new TGraphErrors("../dati/form.in");
    auto *gr2 = new TGraphErrors("../dati/b_sh_s.in");
    auto *gr3 = new TGraphErrors("../dati/b_sh_ns.in");
	spice(*gr2);
	spice(*gr3);
 
	//styles
    gr1->SetMarkerStyle(20);
    gr2->SetMarkerStyle(21);
    gr2->SetMarkerSize(0.0001);
	gr3->SetMarkerStyle(22);
	gr3->SetMarkerSize(0.0001);

	
	
    gr1->SetMarkerColor(kBlue);
    gr2->SetMarkerColor(kRed);
    gr3->SetMarkerColor(kGreen+3);
	
	


    float inf=0.1;
    float sup=2e3;
    // Define the fit function
    TF1 *fit1 = new TF1("ex1", fitd, inf, sup, 2);
    TF1 *fit2 = new TF1("ex1", fitd, inf, sup, 2);
    TF1 *fit3 = new TF1("ex1", fitd, inf, sup, 2);
    fit1->SetParNames("ft_{CR}","ft_{RC}");
    //fit1->SetParNames("f^{sh}_{t}");
    fit2->SetParNames("ft_{CR}","ft_{RC}");
    //fit2->SetParNames("f^{sh}_{t}");  
	fit3->SetParNames("ft_{CR}","ft_{RC}");
	//fit3->SetParNames("f^{sh}_{t}");

    fit1->SetParameters(11,11);
    fit2->SetParameters(11,11);
    fit3->SetParameters(11,11);

    fit1->SetLineColor(kBlue);
	fit2->SetLineColor(kRed);
	fit3->SetLineColor(kGreen+3);

	//set number of points for  logscale compensation
	fit1->SetNpx(1e6);
	fit2->SetNpx(1e6);
	fit3->SetNpx(1e6);



    //TFitResultPtr dati_fit = 
    gr1->Fit(fit1);
	auto *res1 = residui(gr1,fit1);
	gr2->Fit(fit2);
    auto *res2 = residui(gr2,fit2);
	gr3->Fit(fit3);
	auto *res3 = residui(gr3,fit3);

	
	tau(fit1->GetParameter(0),fit1->GetParError(0));
	tau(fit1->GetParameter(1),fit1->GetParError(1));
	

		
    auto *grall = new TMultiGraph();

	grall->Add(gr1);
	grall->Add(gr2);
	grall->Add(gr3);

    grall->GetXaxis()->SetTitle("f [Hz]");
    grall->GetYaxis()->SetTitle("G [dB]");

   
    // Draw the first graph on the first pad
    pad1->cd();
    grall->Draw("AP");
    // fit2->Draw("same");
    // fit3->Draw("same");

    fit1->SetLineColor(kBlack);
    //auto legend = new TLegend(0.3132832,0.1760369,0.6704261,0.4359447,NULL,"brNDC");   
    auto legend = new TLegend(0.2932331,0.1612903,0.7105263,0.5465438,NULL,"brNDC");
    legend->AddEntry(fit1,"Fit #frac{f f_{RC}}{#sqrt{#left( ( f_{CR} f_{RC} - f^{2})^{2} + f^{2}  (f_{CR} + f_{RC})^{2} }} #right)");
    //legend->AddEntry(fit1,"Fit #frac{ff_{t}}{f^{2}+f_{t}^{2}}");
    legend->AddEntry(gr1,"Misure con formatore");
    legend->AddEntry(res2,"Simulazione con LTspice con sonda");
    legend->AddEntry(res3,"Simulazione con LTspice");
    legend->Draw();


    // Hide the X-axis on the first pad
    //pad1->GetFrame()->SetX1(10);
    //pad1->GetFrame()->SetX2(450);
    //gr1->GetXaxis()->SetLabelOffset(999);
    //gr1->GetXaxis()->SetTitleOffset(999);

    // Create the residuals graph
    auto *baseline = new TF1("baseline", "0", inf,sup);
    baseline->SetLineColor(kBlack);

	auto *resall = new TMultiGraph();
    resall->Add(res1);
    resall->Add(res2);
    resall->Add(res3);


    resall->GetXaxis()->SetTitle("f [kHz]");
    resall->GetYaxis()->SetTitle("Residuals");
    
    resall->GetXaxis()->SetLabelSize(0.08); // Adjust the label size as needed
    resall->GetXaxis()->SetTitleSize(0.1); // Adjust the title size as needed
    resall->GetYaxis()->SetLabelSize(0.08); // Adjust the label size as needed
    resall->GetYaxis()->SetTitleSize(0.1); // Adjust the title size as needed

    // Draw the residuals graph on the second pad
    pad2->cd();
	resall->Draw("AP");
    baseline->Draw("same");

    // Synchronize the X-axis of both pads
    pad1->SetBottomMargin(0);
    pad2->SetTopMargin(0);
    pad2->SetBottomMargin(0.3);

 	//set limits for logscale compensation
 	grall->GetXaxis()->SetLimits(inf,sup);
 	resall->GetXaxis()->SetLimits(inf,sup);

 
	


 	pad1->SetGrid();
 	pad1->SetLogx();
 	pad2->SetGrid();
 	pad2->SetLogx();  
 	
	
	

		   // Set style options for the fit
		    gStyle->SetOptFit(10111);
		       // Update the canvas
		    c1->Update();

			float a = 0.37;
			float b = 0.12;
		    
			float l = a-b;
		    // Adjust the fit stats boxes
		    TPaveStats *stats1 = (TPaveStats *)gr1->FindObject("stats");
		    stats1->SetX1NDC(b); // 
		    stats1->SetY1NDC(0.004);
		    stats1->SetX2NDC(a);
		    stats1->SetY2NDC(0.15);
		    stats1->SetLineColor(kBlue);
   			stats1->SetLineWidth(3);

 			a=l+b+0.01;
			b=a+l;
			
		    TPaveStats *stats2 = (TPaveStats *)gr2->FindObject("stats");
		   	stats2->SetX1NDC(b);
		    stats2->SetY1NDC(0.004);
		    stats2->SetX2NDC(a);
		    stats2->SetY2NDC(0.15);
		   	stats2->SetLineColor(kRed);
   			stats2->SetLineWidth(3);

			a=b+0.01;
			b=a+l;
			
 
			l=0.37+l+0.1;
		    TPaveStats *stats3 = (TPaveStats *)gr3->FindObject("stats");
		 	stats3->SetX1NDC(b);
		    stats3->SetY1NDC(0.004);
		    stats3->SetX2NDC(a);
		    stats3->SetY2NDC(0.15);
		    stats3->SetLineColor(kGreen+3);
   			stats3->SetLineWidth(3);
	
		
		    gPad->Modified(); gPad->Update();
			c1->Update();
 

 
}
