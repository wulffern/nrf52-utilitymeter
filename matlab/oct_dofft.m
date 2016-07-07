%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Foct_dofft.m
%% Description:   
%% Author:        Carsten Wulff <wulff@iet.ntnu.no>
%% Created at:    Fri Sep 29 14:56:11 2006
%% Modified at:   Wed Mar 26 07:04:18 2008
%% Modified by:    Carsten Wulff <wulff@iet.ntnu.no>
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%function [y,sndr,menob,fbin,Amp]=dofft(x,x_amp,hann,doplot,fs,txt)
% y: FFT Magnitude normalized to full scale
% sndr: signal to noise and distortion ratio
% menob: effective number of bits
% fbin: Which frequency bin the signal is in
% x: data input
% x_amp: full scale amplitude that the FFT should be normalized to
% hann: use a hanning window (hann=1), do not use a window (hann=0)
% doplot: show plot (doplot=1), do not show plot (doplot=0)
% fs: Sampling frequency, set to 1 for normalized frequency
% txt: Text to append to the plot title
function [y,sndr,menob,fbin,Amp,figure1]=oct_dofft(x,x_amp,hann,doplot,fs,txt)

%Resize down to 2^x points
mod(log2(length(x)),1) 
if mod(log2(length(x)),1) ~= 0 
  M = length(x)
  N = 2^floor(log2(M));
  x = x(M-N+1:M);
end

%Remove DC
x = x - mean(x);

%Get length
N = length(x);

%Apply a hanning window if wanted
if hann==1
  w = x .*hanning(N)';
else
  w = x; 
end

%Do the FFT and extract 0->Nyquist, adjust for the power from Nyquist
%to FS
y = abs(fft(w));
y(1) = 0;
y = y(1:N/2+1)*2;

%Normalize to maximum power
if hann==1
  sigpow = 1.2;
  y = y/(x_amp*N/2);
else
  sigpow = 1;
  y = y/(x_amp*N);
end

%Find the signal frequency, applies correction for hanning window
[u,fbin] = max(y);
spec = y;
fbin = fbin
if fbin < 3
  fbin = 3;
end

if fbin > length(y)-10
  fbin = length(y) - 10;
end

if hann==1
  spread = [ -1 0 1];
else
  spread = [0];
end
signalBins = fbin+spread

%Convert to dBFs
ya = 20*log10(y);

%Find the harmonics
sp = (1:100);
myind = mod((fbin-1).*sp,N);
k=1;
harmonics = ones(1,length(sp)*length(spread));
for i=1:length(myind)
  if(myind(i) > N/2)    
    myind(i) = N - myind(i);
  elseif(myind(i) == 0)
    myind(i) = myind(i) + 1;
  end
  for z=1:length(spread)
    harmonics(k) = myind(i)+spread(z)+1;
    if harmonics(k) == 0 || harmonics(k) > length(y)
      harmonics(k) = 1
    end
    k = k+1;
  end
  
end



%Remove the signal from the noise bins
noiseBins = 1 : length(y);
noiseBins(signalBins) = [];

noharmBins = 1:length(y);
noharmBins(harmonics) = [];

%Calculate signal power
s = norm(spec(signalBins))

%Calculate noise power
n = norm(spec(noiseBins));

h = norm(spec(noharmBins));




try
if n== 0
  sndr = Inf
  menob = 0
  nsndr=0;
  isnr = 0;
  menob = 0;
else
  nsndr = 20*log10(sigpow/n);
  sndr = 20 * log10(s/n);
  isnr = 20*log10(sigpow/h);
  menob = (sndr-1.76)/6.02;
end
catch
  nsndr=0;
  sndr = 0;
  isnr = 0;
  menob = 0;
end

myind = myind+1;

%Generate X
x = linspace(0,0.5,length(y))*fs;

%Find signal and harmonics amplitude
Amp = ya(myind);

if doplot > 0 
% Create figure

%MATLAB
%figure1 = figure('XVisual',...
%    '0x22 (TrueColor, depth 24, RGB mask 0xff0000 0xff00 0x00ff)',...
%    'Color',[1 1 1],'Name','FFT');

%OCTAVE
figure1 = figure(   'Color',[1 1 1]);

% Create axes
axes1 = axes('Parent',figure1,'YGrid','on','XGrid','on');
box('on');
hold on;
axis([ 0 max(x) -140 0])

% Create plot
plot(x,ya,'Parent',axes1,'LineWidth',2,...
    'Color',[0 0 0]);


%Works in MATLAB
% Create scatter
%scatter(x(myind),ya(myind),'MarkerFaceColor',[1 1 1],'MarkerEdgeColor',[0 0 0],...
%    'Marker','diamond',...
%    'DisplayName','Harmonics 1-20',...
%    'Parent',axes1);

% Create xlabel
xlabel('Sampling Frequency [Hz]');

% Create ylabel
ylabel('Magnitude [dB]');

%Works in MATLAB
% Create legend
%legend1 = legend(axes1,'show');
%set(legend1,'Location','SouthEast');

title(['As=',num2str(max(ya)),' ENOB=',num2str(menob),' SNDR=', ...
       num2str(sndr), '  ISNDR= ',num2str(nsndr), ...
       '  ISNR=',num2str(isnr),'  f0=',num2str(fbin/N*fs/1e6),'MHz',txt]);

end
