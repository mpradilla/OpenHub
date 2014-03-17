package org.twdata.maven.cli;

import java.io.File;
import java.lang.reflect.Constructor;
import java.lang.reflect.Method;
import java.util.List;
import java.util.Properties;

import org.apache.maven.Maven;
import org.apache.maven.artifact.repository.ArtifactRepository;
import org.apache.maven.execution.DefaultMavenExecutionRequest;
import org.apache.maven.execution.MavenExecutionRequest;
import org.apache.maven.execution.MavenSession;
import org.apache.maven.monitor.event.EventDispatcher;
import org.apache.maven.profiles.DefaultProfileManager;
import org.apache.maven.profiles.ProfileManager;
import org.apache.maven.project.MavenProject;
import org.apache.maven.settings.Settings;
import org.twdata.maven.cli.console.CliConsole;

public class PhaseCallRunner {
    private final MavenSession session;
    private final MavenProject project;
    private final File userDir;
    private boolean pluginExecutionOfflineMode;

    public PhaseCallRunner(MavenSession session, MavenProject project) {
        this.session = session;
        this.project = project;
        this.userDir = new File(System.getProperty("user.dir"));
        this.pluginExecutionOfflineMode = session.getSettings().isOffline();
    }

    public boolean run(MavenProject currentProject, PhaseCall phaseCall, CliConsole console) {
        try {
            // QUESTION: which should it be?
            session.getExecutionProperties().putAll(phaseCall.getProperties());
            //project.getProperties().putAll(commandCall.getProperties());

            session.setCurrentProject(currentProject);
            session.getSettings().setOffline(phaseCall.isOffline() ? true : pluginExecutionOfflineMode);
            ProfileManager profileManager = new DefaultProfileManager(session.getContainer(), phaseCall.getProperties());
            profileManager.explicitlyActivate(phaseCall.getProfiles());

            Settings settings = session.getSettings();
            
            Class<DefaultMavenExecutionRequest> merClass = DefaultMavenExecutionRequest.class;
            Constructor[] ctrs = merClass.getConstructors();

            MavenExecutionRequest request = null;
            Method pomMethod = null;
            if(ctrs[0].getParameterTypes().length < 1)
            {
                request = merClass.newInstance();
                
                request.setLocalRepository(session.getLocalRepository())
                       .setOffline( session.isOffline() )
                       .setInteractiveMode( settings.isInteractiveMode() )
                       .setProxies( settings.getProxies() )
                       .setServers( settings.getServers() )
                       .setMirrors( settings.getMirrors() )
                       .setPluginGroups( session.getPluginGroups() )
                       .setGoals( phaseCall.getPhases() )
                       .setSystemProperties( session.getSystemProperties() )
                       .setUserProperties( session.getUserProperties() )
                       .setActiveProfiles( profileManager.getActiveProfiles() );

                pomMethod = request.getClass().getMethod("setPom",File.class);
                pomMethod.invoke(request,new File(currentProject.getBasedir(), "pom.xml"));
            }
            else
            {
                Constructor<DefaultMavenExecutionRequest> ctr = merClass.getConstructor(new Class[]{
                        ArtifactRepository.class
                        ,Settings.class
                        , EventDispatcher.class
                        ,List.class
                        ,String.class
                        ,ProfileManager.class
                        , Properties.class
                        ,Properties.class
                        ,Boolean.TYPE
                });
                
                request = ctr.newInstance(
                        session.getLocalRepository()
                        ,session.getSettings()
                        ,session.getEventDispatcher()
                        ,phaseCall.getPhases()
                        ,userDir.getPath()
                        ,profileManager
                        ,session.getExecutionProperties()
                        ,currentProject.getProperties()
                        ,true
                );

                pomMethod = request.getClass().getMethod("setPomFile",String.class);
                pomMethod.invoke(request,new File(currentProject.getBasedir(), "pom.xml").getPath());
            }

            if (!phaseCall.isRecursive()) {
                request.setRecursive(false);
            }
            
            Maven mvn = (Maven) session.lookup(Maven.class.getName());
            Method execMethod = mvn.getClass().getMethod("execute",MavenExecutionRequest.class);
            
            Class returnClass = execMethod.getReturnType();
            Object result = execMethod.invoke(mvn,request);
            
            if(returnClass.getSimpleName().equals("MavenExecutionResult"))
            {
                Method prjMethod = result.getClass().getMethod("getProject");
                MavenProject resultProject = (MavenProject) prjMethod.invoke(result);
                
                Method bsMethod = result.getClass().getMethod("getBuildSummary",MavenProject.class);
                Object summary = bsMethod.invoke(result,resultProject);
                if(summary.getClass().getSimpleName().equals("BuildSuccess"))
                {
                    console.writeInfo("------------------------------------------------------------------------");
                    console.writeInfo("BUILD SUCCESSFUL");
                    console.writeInfo("------------------------------------------------------------------------");
                }
                else
                {
                    console.writeInfo("------------------------------------------------------------------------");
                    console.writeInfo("BUILD ERROR");
                    console.writeInfo("------------------------------------------------------------------------");
                    return false;
                }
            }
            
            console.writeInfo("Current project: " + project.getArtifactId());
            return true;
        } catch (Exception e) {
            console.writeError("Failed to execute '" + phaseCall.getPhases() + "' on '"
                    + currentProject.getArtifactId() + "'");
            return false;
        }
    }
}
