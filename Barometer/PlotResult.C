void PlotResult(void)
{
    /*
     * Pressure, altitude and density. 
     *
     * https://www.mide.com/air-pressure-at-altitude-calculator
     * https://en.wikipedia.org/wiki/Barometric_formula
     * https://community.bosch-sensortec.com/t5/Question-and-answers/How-to-calculate-the-altitude-from-the-pressure-sensor-data/qaq-p/5702
     *
     * H = 44330 * [1 - (P/p0)^(1/5.255) ]
     */
    TCanvas *Hobbes = new TCanvas("Dist","A test",5,5,1200,600);
    Hobbes->cd();
    TPad    *Calvin = new TPad("Calvin","Silly",0.02,0.02,0.99,0.99, 33);
    Calvin->Draw();
    Calvin->cd();
    Calvin->SetGrid();

    //TGraph *tg = new TGraph("Barometer_240607.log","%lg,%lg");
    TGraph *tg = new TGraph("baro.log","%lg,%lg");
    tg->Draw("ALP");

    tg->GetXaxis()->SetTimeDisplay(1);
    tg->GetXaxis()->SetNdivisions(513);
    tg->GetXaxis()->SetTimeFormat("%d %H:%M:%S");
    tg->GetXaxis()->SetTimeOffset(0,"gmt");

    TH1 *hbs = tg->GetHistogram();
    hbs->SetXTitle("Time");
    hbs->SetYTitle("Pressure (mb)");
    hbs->SetLabelSize(0.03,"X");
    hbs->SetLabelSize(0.03,"Y");
}
Double_t HeightFromPessure(Double_t P, Double_t P0)
{
    // Simple version
    Double_t H = 44330.0 * (1.0-pow(P/P0, 1.0/5225.0)); // in meters
    return H;
}

const Double_t h_b = 0.0;
const Double_t T_b = 288.0;
const Double_t L_b = -0.0065;
const Double_t M   = 0.0289644;
Double_t HeightFromPessure2(Double_t P, Double_t P0)
{
    /*
     * https://www.mide.com/air-pressure-at-altitude-calculator
     *
     * h = h_b + T_b/L_b *[(P/P_b)^(R *L_b/(g_0 * M)) -1]
     *
     * P_b = sea level static pressure [Pa] (1013.25mbar = 101325.0 )
     * T_b = temperature at sea level [K] = 288.0K
     * L_b = standard temperature lapse rate [K/m] = -0.0065
     * h   = height above sea level [m]
     * h_b = height at bottom of atmospheric layer [m] 
     * R = universal gas constant 8.31432 [N-m/(mol K)]
     * g_0 = gravitational acceleration consant = 9.80665 [m/s^2]
     * M   = molar mass of Earth's air = 0.0289644 [kg/mol]
     * 
     */

    Double_t h = h_b+T_b/L_b*(pow(P/P0,TMath::R()*L_b/(TMath::Gn()*M)-1.0));
    return h;
}
#if 0
Double_t PressureFromHeight(Double_t h)
{
    Double_t k1 = 1.0 + L_b/T_b * (h-hb);
    Double_t N  = -TMath::Gn() * M/TMath::R()/TMath::L_b;
    Double_t P = Pb * pow(k1,N)
    return P;
}
#endif
