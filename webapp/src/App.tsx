import React from 'react';
import TitleBar from './components/TitleBar';
import HomePage from './pages/home';
import { AppSidebar } from './components/sidebar';
import { SidebarProvider, SidebarInset } from '@/components/ui/sidebar';
import '@fontsource-variable/inter/wght.css';
const AppContent: React.FC = () => {
  return (
    <div className="h-screen w-screen flex flex-col overflow-hidden dark bg-background ">
      <div className="fixed top-0 left-0 right-0 z-50">
        <TitleBar />
      </div>
      <div className="flex-1 flex overflow-hidden" style={{ marginTop: '40px' }}>
        <SidebarProvider defaultOpen={true} className=''>
          <AppSidebar className='' />
          <SidebarInset className="flex-1 overflow-hidden">
            <HomePage />
          </SidebarInset>
        </SidebarProvider>
      </div>
    </div>
  );
};

const App: React.FC = () => {
  return (
    <AppContent />
  );
};

export default App;
