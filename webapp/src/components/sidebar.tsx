import * as React from "react"
import {
    Home,
    FileText,
    Folder,
    Settings,
    Search,
    Star,
    Clock,
    Trash2,
    Cloud,
} from "lucide-react"

import {
    Sidebar,
    SidebarContent,
    SidebarGroup,
    SidebarHeader,
    SidebarMenu,
    SidebarMenuButton,
    SidebarMenuItem,
    SidebarFooter,
    SidebarInput,
} from "@/components/ui/sidebar"
import { Badge } from "@/components/ui/badge"

// Navigation data
const data = {
    navMain: [
        {
            title: "Home",
            icon: Home,
            url: "#",
            isActive: true,
        },
        {
            title: "Documents",
            icon: FileText,
            url: "#",
            badge: 12,
        },
        {
            title: "Projects",
            icon: Folder,
            url: "#",
            badge: 5,
        },
        {
            title: "Favorites",
            icon: Star,
            url: "#",
        },
        {
            title: "Recent",
            icon: Clock,
            url: "#",
        },
        {
            title: "Cloud Storage",
            icon: Cloud,
            url: "#",
        },
        {
            title: "Trash",
            icon: Trash2,
            url: "#",
        },
    ],
}

export function AppSidebar({ ...props }: React.ComponentProps<typeof Sidebar>) {
    const [activeItem, setActiveItem] = React.useState("Home")

    return (
        <Sidebar variant="sidebar" collapsible="icon" className="border-r border-border" {...props}>
            <SidebarHeader className="border-b border-border">
                <SidebarMenu>
                    <SidebarMenuItem>
                        <SidebarMenuButton size="lg" asChild>
                            <a href="#" className="flex items-center gap-2">
                                <div className="bg-muted flex aspect-square size-8 items-center justify-center rounded-lg">
                                    <Folder className="size-4" />
                                </div>
                                <div className="flex flex-col gap-0.5 leading-none">
                                    <span className="font-semibold">Workspace</span>
                                    <span className="text-xs text-muted-foreground">My Files</span>
                                </div>
                            </a>
                        </SidebarMenuButton>
                    </SidebarMenuItem>
                </SidebarMenu>
            </SidebarHeader>

            <SidebarContent className="overflow-hidden">
                {/* Search */}
                <SidebarGroup>
                    <div className="px-3 py-2">
                        <div className="relative">
                            <Search className="absolute left-2 top-1/2 h-4 w-4 -translate-y-1/2 text-muted-foreground" />
                            <SidebarInput
                                placeholder="Search..."
                                className="pl-8 h-9"
                            />
                        </div>
                    </div>
                </SidebarGroup>

                {/* Navigation Items */}
                <SidebarGroup>
                    <SidebarMenu className="gap-1 px-2">
                        {data.navMain.map((item) => (
                            <SidebarMenuItem key={item.title}>
                                <SidebarMenuButton
                                    asChild
                                    isActive={activeItem === item.title}
                                    onClick={() => setActiveItem(item.title)}
                                    tooltip={item.title}
                                    className="group"
                                >
                                    <a href={item.url} className="flex items-center gap-3">
                                        <item.icon className="size-4" />
                                        <span className="flex-1">{item.title}</span>
                                        {item.badge && (
                                            <Badge
                                                variant="secondary"
                                                className="ml-auto h-5 px-1.5 text-xs font-medium"
                                            >
                                                {item.badge}
                                            </Badge>
                                        )}
                                    </a>
                                </SidebarMenuButton>
                            </SidebarMenuItem>
                        ))}
                    </SidebarMenu>
                </SidebarGroup>
            </SidebarContent>

            <SidebarFooter>
                <SidebarMenu>
                    <SidebarMenuItem>
                        <SidebarMenuButton asChild tooltip="Settings">
                            <a href="#" className="flex items-center gap-3">
                                <Settings className="size-4" />
                                <span>Settings</span>
                            </a>
                        </SidebarMenuButton>
                    </SidebarMenuItem>
                </SidebarMenu>
            </SidebarFooter>
        </Sidebar>
    )
}
