import { createRoot } from 'react-dom/client'
import './index.css'
import App from './App.tsx'

// Force dark theme
document.documentElement.classList.add('dark')
document.documentElement.style.colorScheme = 'dark'

const root = document.getElementById('root')!
const reactRoot = createRoot(root)

reactRoot.render(<App />)
