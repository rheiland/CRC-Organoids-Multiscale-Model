%Net Export rate calculations
clear all
close
clc

Well_Volume = 1.5; %ml
Well_Volume = Well_Volume/1000; %l

glu_con = 17.5; %mM


glu_amount = Well_Volume * glu_con ; %mmol


time = 24; %hr
integrated_cell_count = 500000; %cells ???

volume_of_cell = 1000; %um3/cell
volume_of_cell = 1000/1e15; %l/cell

glucose_uptake_rate = 0.223; %mM/hr = mmol/l/hr

uptaken_glucose_conc_by_rate = glucose_uptake_rate * time; %mM = mmol/l

glucose_uptake_flux = glucose_uptake_rate * volume_of_cell; %mmol/cell/hr
uptaken_glucose_amount_by_flux = glucose_uptake_flux * time * integrated_cell_count; %mmol


e_0 = glucose_uptake_flux/glu_con;


