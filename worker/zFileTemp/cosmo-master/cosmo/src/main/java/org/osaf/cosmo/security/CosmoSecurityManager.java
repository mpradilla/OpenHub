/*
 * Copyright 2005-2007 Open Source Applications Foundation
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package org.osaf.cosmo.security;

import org.osaf.cosmo.model.Item;

/**
 * Rrepresents a server-wide security controller for Cosmo. It
 * provides entry points for obtaining information about the
 * authentication state of the currently executing thread or for
 * initiating authentication (or overwriting the current state).
 *
 * @see CosmoSecurityContext
 */
public interface CosmoSecurityManager {

    /**
     * Provides a <code>CosmoSecurityContext</code> representing a
     * previously authenticated principal.
     */
    public CosmoSecurityContext getSecurityContext()
        throws CosmoSecurityException;

    /**
     * Authenticates the given Cosmo credentials and registers a
     * <code>CosmoSecurityContext</code> for them. This method is used
     * when Cosmo components need to programatically log in a user
     * rather than relying on a security context already being in
     * place.
     */
    public CosmoSecurityContext initiateSecurityContext(String username,
                                                        String password)
        throws CosmoSecurityException;

    /**
     * Validates that the current security context has the requested
     * permission for the given item.
     *
     * @throws PermissionDeniedException if the security context does
     * not have the required permission
     */
    public void checkPermission(Item item,
                                int permission)
        throws PermissionDeniedException;
}
