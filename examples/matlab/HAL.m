%==================================================================================================
% HAL.m - Matlab wrapper for the USPlatform HAL.
%--------------------------------------------------------------------------------------------------
% Copyright (c) 2019 us4us Ltd. / MIT License
%==================================================================================================
classdef HAL
    properties

    end

    methods (Static = true)
        function configure(json)
            HALmex('configure', json);
        end

        function start()
            HALmex('start');
        end

        function stop()
            HALmex('stop');
        end

        function sync()
            HALmex('sync');
        end

        function halOutput = getData()
            halOutput = HALmex('getData');
        end

		function devicesNames = getAvailableHALDevicesNames()
			devicesNames = HALmex('getAvailableHALDevicesNames');
		end

		function setHALDevice(deviceName)
			HALmex('setHALDevice', deviceName);
        end
    end
end
