import React, { useState, useEffect } from 'react';
import { Button } from '@/components/ui/button';
import { Tooltip, TooltipContent, TooltipProvider, TooltipTrigger } from '@/components/ui/tooltip';
import {
  Minus,
  X,
  Maximize,
  Copy
} from 'lucide-react';

import CloseIcon from '../assets/close.svg'
import MaximizeIcon from '../assets/maximize.svg'
import RestoreIcon from '../assets/restore.svg'
import MinimizeIcon from '../assets/minimize.svg'

interface CefQueryRequest {
  request: string;
  onSuccess: (response: string) => void;
  onFailure: (error_code: number, error_message: string) => void;
}

interface WindowControlsInfo {
  hasNativeControls: boolean;
  useWebControls?: boolean;
  platform: 'windows' | 'macos' | 'linux';
  controlsPosition: 'left' | 'right';
}

declare global {
  interface Window {
    cefQuery?: (request: CefQueryRequest) => void;
  }
}

export default function TitleBar() {
  const [windowControlsInfo, setWindowControlsInfo] = useState<WindowControlsInfo>({
    hasNativeControls: false,
    useWebControls: true,
    platform: 'windows',
    controlsPosition: 'right'
  });
  const [isMaximized, setIsMaximized] = useState(false);

  // CEF IPC helper function
  const sendCefQuery = (request: string): Promise<string> => {
    return new Promise((resolve, reject) => {
      if (window.cefQuery) {
        window.cefQuery({
          request,
          onSuccess: (response: string) => resolve(response),
          onFailure: (error_code: number, error_message: string) => {
            reject(new Error(`CEF Query failed [${error_code}]: ${error_message}`));
          }
        });
      } else {
        reject(new Error('CEF Query not available'));
      }
    });
  };

  useEffect(() => {
    const initWindowControls = async () => {
      try {
        const info = await sendCefQuery('get_window_controls_info');
        const parsedInfo = JSON.parse(info) as WindowControlsInfo;
        setWindowControlsInfo(parsedInfo);

        const maximized = await sendCefQuery('is_window_maximized');
        setIsMaximized(maximized === 'true');
      } catch (error) {
        console.error('Failed to initialize window controls:', error);
      }
    };

    initWindowControls();
  }, []);

  const handleMinimize = async (): Promise<void> => {
    try {
      await sendCefQuery('minimize_window');
    } catch (error) {
      console.error('Failed to minimize window:', error);
    }
  };

  const handleMaximize = async (): Promise<void> => {
    try {
      if (isMaximized) {
        await sendCefQuery('restore_window');
        setIsMaximized(false);
      } else {
        await sendCefQuery('maximize_window');
        setIsMaximized(true);
      }
    } catch (error) {
      console.error('Failed to maximize/restore window:', error);
    }
  };

  const handleClose = async (): Promise<void> => {
    try {
      await sendCefQuery('close_window');
    } catch (error) {
      console.error('Failed to close window:', error);
    }
  };

  // macOS-style traffic light button
  const MacOSButton = ({
    onClick,
    color,
    icon: Icon,
    tooltip
  }: {
    onClick: () => void;
    color: 'red' | 'yellow' | 'green';
    icon: React.ComponentType<any>;
    tooltip: string;
  }) => {
    const colorClasses = {
      red: 'bg-red-500 hover:bg-red-600',
      yellow: 'bg-yellow-500 hover:bg-yellow-600',
      green: 'bg-green-500 hover:bg-green-600'
    };

    return (
      <Tooltip>
        <TooltipTrigger asChild>
          <button
            onClick={onClick}
            className={`w-3 h-3 rounded-full ${colorClasses[color]} transition-all duration-200 flex items-center justify-center group relative`}
            style={{ WebkitAppRegion: 'no-drag' } as React.CSSProperties}
          >
            <Icon className="h-2 w-2 text-white opacity-0 group-hover:opacity-100 transition-opacity absolute" />
          </button>
        </TooltipTrigger>
        <TooltipContent side="bottom">
          <p>{tooltip}</p>
        </TooltipContent>
      </Tooltip>
    );
  };

  // Windows/Linux-style button
  const WindowsButton = ({
    onClick,
    iconSrc,
    tooltip,
    isClose = false
  }: {
    onClick: () => void;
    iconSrc: string;
    tooltip: string;
    isClose?: boolean;
  }) => {
    return (
      <Tooltip>
        <TooltipTrigger asChild>
          <Button
            onClick={onClick}
            variant="ghost"
            size="icon"
            className={`h-8 w-8 rounded-none ${isClose ? 'hover:bg-red-600 hover:text-white' : 'hover:bg-accent'}`}
            style={{ WebkitAppRegion: 'no-drag' } as React.CSSProperties}
          >
            <img src={iconSrc} alt={tooltip} className="h-2.5 w-2.5" />
          </Button>
        </TooltipTrigger>
        <TooltipContent side="bottom">
          <p>{tooltip}</p>
        </TooltipContent>
      </Tooltip>
    );
  };

  return (
    <TooltipProvider delayDuration={300}>
      <div
        className="h-10 w-full flex bg-transparent border-border items-center justify-end shrink-0 select-none"
        style={{ WebkitAppRegion: 'drag' } as React.CSSProperties}
      >
        {/* Right Section: Window Controls */}
        {windowControlsInfo.useWebControls && (
          <div className="flex items-center h-full" style={{ WebkitAppRegion: 'no-drag' } as React.CSSProperties}>
            {windowControlsInfo.controlsPosition === 'left' && windowControlsInfo.platform === 'macos' ? (
              // macOS-style traffic lights
              <div className="flex items-center gap-2 px-3">
                <MacOSButton
                  onClick={handleClose}
                  color="red"
                  icon={X}
                  tooltip="Close"
                />
                <MacOSButton
                  onClick={handleMinimize}
                  color="yellow"
                  icon={Minus}
                  tooltip="Minimize"
                />
                <MacOSButton
                  onClick={handleMaximize}
                  color="green"
                  icon={isMaximized ? Copy : Maximize}
                  tooltip={isMaximized ? "Restore" : "Maximize"}
                />
              </div>
            ) : (
              // Windows/Linux-style controls using SVG assets
              <div className="flex items-center h-full">
                <WindowsButton
                  onClick={handleMinimize}
                  iconSrc={MinimizeIcon}
                  tooltip="Minimize"
                />
                <WindowsButton
                  onClick={handleMaximize}
                  iconSrc={isMaximized ? RestoreIcon : MaximizeIcon}
                  tooltip={isMaximized ? "Restore" : "Maximize"}
                />
                <WindowsButton
                  onClick={handleClose}
                  iconSrc={CloseIcon}
                  tooltip="Close"
                  isClose={true}
                />
              </div>
            )}
          </div>
        )}

        {/* Reserve space for native controls if they exist */}
        {windowControlsInfo.hasNativeControls && !windowControlsInfo.useWebControls && (
          <div
            className="h-full"
            style={{
              width: windowControlsInfo.controlsPosition === 'left' ? '80px' : '140px',
              WebkitAppRegion: 'no-drag'
            } as React.CSSProperties}
          />
        )}
      </div>
    </TooltipProvider>
  );
}