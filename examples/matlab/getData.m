%==================================================================================================
% getData.m - A simple Matlab script for ultrasound data acquisition with the USPlatform.
%--------------------------------------------------------------------------------------------------
% Copyright (c) 2019 us4us Ltd. / MIT License
%==================================================================================================
path(path, 'USPlatform/lib');   % add path to the USPlatform SDK libs/DLLs

%% Setup the USPlatform
json = fileread('PA-BFR.json'); % load TX/RX JSON config file

HAL.configure(json);    # init HAL with the TX/RX JSON
HAL.start();            # start TX/RX

[B,A] = butter(5, 1/25, 'high');    % define a RF high-pass filter

%% Acquire ultrasound data Frame-by-Frame
while(1)
    signal = single(HAL.getData());
    HAL.sync();
    %figure(1);     % show raw RF data
    %signal_filt = filter(B, A, signal, [], 2); % filter raw RF with HP filter

    plot(signal(:,:)'); % plot RF signal
    %drawnow;
    %pause(0.1);
end
