%% LININT - linear interpolation function
%% Interpolate the time-data matrix TU, given in TDATA format, for 
%% the input U at times TI, where TI may be a vector.  The interpolation
%% gives continuity from the right at discontinuous points of TU (see
%% TDATA format on how to specify discontinuities).
%% 
%% The output is the interpolated value of the input profile TU 
%% at times TI where:
%%
%% UOUT = [U(TI(1)) U(TI(2)) ... U(TI(N))],      N = LENGTH(TI)
%%
%% If TI(i) is outside the time range of TU, only a warning is given and
%% the closest time value in the time range of TU is taken for obtaining U.
%%
%% UOUT = LININT(TU,TI)

function uout = linint(tu,ti)

if (size(tu,2) > 1)
	uout = [];
	for i=1:length(ti)
		if ((ti(i) < tu(1,1)) | (ti(i) > tu(size(tu,1),1)))
			%disp('WARNING - interpolation time TI(i) outside TU time range in LININT');
		end
		uout = [uout helperfun(tu,ti(i))];
	end
else
	uout = [];
	disp('ERROR - bad time-data format for TU in LININT')
end

%% Helper function
function u = helperfun(tuin,t)
	
idx = size(tuin,2);

if (size(tuin,1) > 1)
	i = 1;
	while ((tuin(i,1) <= t) & (i < size(tuin,1)))
		i = i + 1;
	end

	%% t is greater or equal to the last time value in tuin
	if (tuin(i,1) < t)
		ti = tuin(i,1);
	else
		ti = t;
	end
	
	
	if (i > 1)
		if ((tuin(i,1)-tuin(i-1,1)) ~= 0)
			u = tuin(i-1,2:idx) +...
			    (tuin(i,2:idx)-tuin(i-1,2:idx))/...
				(tuin(i,1)-tuin(i-1,1))*(ti-tuin(i-1,1));
		
			u = u.';
		%% Only occurs when last interval is of zero length
		%% -> take last value
		else
			u = tuin(i,2:idx).';
		end
	%% t is less than the first time value in tuin
	else
		u = tuin(1,2:idx).';
	end
else
	u = tuin(1,2:idx).';
end