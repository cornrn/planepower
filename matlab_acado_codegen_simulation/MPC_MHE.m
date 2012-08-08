clear all
% close all
clc

addpath('Simulation','Matlabfunctions','code_export_MPC','code_export_MHE')

recompileMHE = 1;
recompileMPC = 1;

generateCodeForOrocos = 0;


% MPC settings
% NMPC Sim.W0 MPC.Ncvp MPC.Tc 1 MPC.Ref.r

mpc_settings
Sim.W0 = W0;         % Wind known by the controller
MPC.Ncvp = Ncvp; % number of cvp
MPC.Tc = Tc; % horizon in seconds
MPC.Ref.r = r;            % tether length

MPC.Ts  = MPC.Tc/MPC.Ncvp; % sampling time
MPC.Nref = MPC.Ncvp+1; % 500;% % number of elements in the reference file
% Multiplying factor for the terminal cost
MPC.Sfac = 1; % factor multiplying the terminal cost
MPC.is_init = 0;

% MHE settings
MHE.Tc = 1;
MHE.Ncvp = 10;
MHE.Ts  = MHE.Tc/MHE.Ncvp;
MHE.Nref = MHE.Ncvp+1;%500;%

% Consistency check for the sampling times in MPC and MHE
if MPC.Ts ~= MHE.Ts
    error('Sampling times in MPC and MHE do not match')
end

% Set integration parameters
Sim.intoptions.AbsTol = 1e-8;
Sim.intoptions.RelTol = 1e-8;
Sim.intoptions.MaxStep = 1/10;
Sim.intoptions.Events = 'off';

% Simulation settings
Sim.Tf = 8;                 % simulation final time
Sim.decoupleMPC_MHE = 0;    % Set different from 0 to have MPC and MHE running in parallel, but independently, i.e.: MPC gets perfect state estimates

% Wind parameters
Sim.DeltaW = 0;     % Windgusts or unknown wind

% Sensor noise
Sim.Noise.is = 1;           % set to 0 to kill the noise
Sim.Noise.factor = 1/40;    % noise: factor*scale_of_related_state = max_noise

% Initial condition
MPC.Ref.z = -0.1189362777884522 ;         % reference trajectory height (relative to the arm)
MPC.Ref.delta = 0;          % initial carousel angle
MPC.Ref.RPM = 60;           % carousel rotational velocity
MPC.Ref.ddelta = 2*pi*MPC.Ref.RPM/60.;
Sim.r = MPC.Ref.r;          % copy the tether length (just for the ease of use)

% Define the reference by stacking data in time vectors
for k = 1:(Sim.Tf/MPC.Ts + 2)
    MPC.Ref.time(k,1) = MPC.Ts*(k-1);
    MPC.Ref.vars(k,1:3)   = [MPC.Ref.z MPC.Ref.r MPC.Ref.ddelta];
    
    % apply a step change after 1 second
    if MPC.Ref.time(k,1) >= 1 && MPC.Ref.time(k,1) < 5 
        MPC.Ref.z = 0.1;
    elseif MPC.Ref.time(k,1) >= 5
        MPC.Ref.z = 0.0;
    end
end

display('------------------------------------------------------------------')
display('               Initialization')
display('------------------------------------------------------------------')

% Initialize (first MPC, then MHE)
[MPC,Sim] = initializeMPC(MPC,Sim);

% Pass the MPC initialization to the MHE to allow for warm start
MHE.X0 = MPC.X0;
MHE.U0 = MPC.Uref(1,:);

MHE = initializeMHE(MHE,Sim);

% return

%% Generate the code

%{
	Call the makefile which will build:
		- NMPC
		- MHE
		_ eq/equilibrium
		- eq_MHE/equilibrium

	NOTE: This guy internalyl calles CMake
%}
!make

if recompileMHE
    % MHE compilation is not yet fully automatic...
	
	display(['!./MHE ',num2str(Sim.W0),'     ',num2str(MHE.Ncvp),'     ',num2str(MHE.Tc),'     ',num2str(1),'     ',num2str(MPC.Ref.r)]);
    eval(['!./MHE ',num2str(Sim.W0),'     ',num2str(MHE.Ncvp),'     ',num2str(MHE.Tc),'     ',num2str(1),'     ',num2str(MPC.Ref.r)]);
    
	if (generateCodeForOrocos == 0)
		cd code_export_MHE/
		make_mex
		cd ..
	end;
end

if recompileMPC
    % Compile the ACADO NMPC code
%     !make NMPC
    
    % Export the code
    display(['!./NMPC ',num2str(Sim.W0),'     ',num2str(MPC.Ncvp),'     ',num2str(MPC.Tc),'     ',num2str(1),'     ',num2str(MPC.Ref.r)]);
	% NMPC Sim.W0 MPC.Ncvp MPC.Tc 1 MPC.Ref.r
    eval(['!./NMPC ',num2str(Sim.W0),'     ',num2str(MPC.Ncvp),'     ',num2str(MPC.Tc),'     ',num2str(1),'     ',num2str(MPC.Ref.r)]);
    
    % Compile to generate the mex
	if (generateCodeForOrocos == 0)
		cd code_export_MPC/
		make_mex
		cd ..
	end;
end	

if ( generateCodeForOrocos )
	disp('Code-generation for OROCOS -- done');
	
	return;
end;

%% %%%%%%%%%%%%%%%%%%%%%%% SIMULATION LOOP %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

display('------------------------------------------------------------------')
display('               Simulation Loop')
display('------------------------------------------------------------------')

Sim.time = 0;

% Tis few lines of code are essentially useless, they are there just in
% case we want to test if scaling up-down the cost function affects the
% solution or not!
% 
% factor = 1e0;
% MHE.QQ = MHE.QQ*factor;
% MHE.Qend = MHE.Qend*factor;
% 
% factor2 = 1e0;
% MPC.Q=MPC.Q*factor2;
% MPC.R=MPC.R*factor2;
% MPC.S=MPC.S*factor2;


MHETIME = [];
while (Sim.time(end) < Sim.Tf)
    
    % Solve MHE OCP
    tic
    [MHE.U,MHE.X,MHE.stepKKT] = MHEstep(MHE.Xguess,MHE.Uguess,MHE.Measurements(1:end-1,2:end),MHE.Measurements(end,14:end-3),MHE.QQ,MHE.Qend);
    MHEtime = toc
    MHETIME = [MHETIME;MHEtime];
    
    % Save the MHE step
    MHE = getMHE(MHE,Sim);
    
    % Get the estimated state and pass it to MPC
    MPC = getEstimatedState(MHE,MPC,Sim);
    
    % Solve NMPC OCP
    tic 
    [MPC.U,MPC.X,MPC.stepKKT] = MPCstep(MPC.X0,MPC.Xguess,MPC.Uguess,MPC.Xref(2:end,:),MPC.Uref(1:end-1,:),MPC.Q,MPC.R,MPC.S);
    toc
    
    
    [MPC,Sim] = getMPC(MPC,Sim);
    
    % Get a new measurement
    MHE = newMeasurement(MHE,MPC,Sim);
    
    Sim.time(end)
end

plotFigures(MHE,MPC,Sim);

Simulation.MPC = MPC;
Simulation.MHE = MHE;
Simulation.Sim = Sim;

MPC = Simulation.MPC;
MHE = Simulation.MHE;
Sim = Simulation.Sim;

%% Save the simulated trajectory and the controls

% Array with time in the first column and the controls in the next ones
tu = Sim.controls;

!rm TU.txt
File = 'TU.txt';
Form = '';
for k = 1:size(tu,2)
    Form = [Form,'%6.16e '];
end
Form = [Form,'\n'];

fid = fopen(File, 'w');
fprintf(fid,Form,tu');
fclose(fid);


% Array with time in the first column and the controls in the next ones
tx = [Sim.time Sim.state];

!rm TX.txt
File = 'TX.txt';
Form = '';
for k = 1:size(tx,2)
    Form = [Form,'%6.16e '];
end
Form = [Form,'\n'];

fid = fopen(File, 'w');
fprintf(fid,Form,tx');
fclose(fid);
