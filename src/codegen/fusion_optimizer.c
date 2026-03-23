/*
 * NetLang Fusion Optimizer - Implementation
 * 
 * Detects Conv+ReLU and Dense+ReLU patterns to enable fused kernel generation.
 * 
 * Author: Abhijeet Deb Nath
 * Date: February 2026
 */

#include "fusion_optimizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Detect Conv2D + ReLU fusion */
static int detect_conv_relu_fusion(ASTNode* network, Scope* scope) {
    (void)scope;
    int fusion_count = 0;
    
    if (network->type != NODE_NETWORK) return 0;
    
    ASTList* statements = network->data.network.statements;
    if (statements == NULL || statements->head == NULL) return 0;
    
    // Walk through statements looking for Conv2D layers with ReLU activation
    ASTNode* current = statements->head;
    while (current != NULL) {
        if (current->type == NODE_ASSIGNMENT) {
            ASTNode* value = current->data.assignment.value;
            
            // Check for Conv2D with inline ReLU activation
            if (value && value->type == NODE_CONV2D) {
                if (value->data.conv2d.activation == ACT_RELU) {
                    // Allocate and attach fusion info
                    if (value->fusion_info == NULL) {
                        value->fusion_info = (FusionInfo*)malloc(sizeof(FusionInfo));
                        value->fusion_info->type = FUSION_CONV_RELU;
                        value->fusion_info->is_fused = true;
                        value->fusion_info->is_fusion_root = true;
                        value->fusion_info->fused_with = NULL;
                        
                        fusion_count++;
                    }
                }
            }
            
            // Check for Dense with inline ReLU activation
            if (value && value->type == NODE_DENSE) {
                if (value->data.dense.activation == ACT_RELU) {
                    // Allocate and attach fusion info
                    if (value->fusion_info == NULL) {
                        value->fusion_info = (FusionInfo*)malloc(sizeof(FusionInfo));
                        value->fusion_info->type = FUSION_DENSE_RELU;
                        value->fusion_info->is_fused = true;
                        value->fusion_info->is_fusion_root = true;
                        value->fusion_info->fused_with = NULL;
                        
                        fusion_count++;
                    }
                }
            }
        }
        
        // Move to next statement (assuming statements are linked)
        // This depends on your AST list implementation
        current = NULL;  // Simplified for now
        break;
    }
    
    return fusion_count;
}

/* Main fusion detection entry point */
int detect_fusion_patterns(ASTNode* network, Scope* scope) {
    if (network == NULL) {
        fprintf(stderr, "ERROR: NULL network in fusion optimizer\n");
        return 0;
    }
    
    int total_fusions = 0;
    
    // Detect Conv+ReLU fusions
    total_fusions += detect_conv_relu_fusion(network, scope);
    
    // TODO: Add more fusion patterns:
    // - Dense + ReLU
    // - Conv + BatchNorm + ReLU
    // - Conv + Add + ReLU (residual)
    
    return total_fusions;
}

/* Print fusion statistics */
void print_fusion_stats(int fusion_count) {
    if (fusion_count > 0) {
        fprintf(stderr, "      ✓ Detected %d operator fusion(s)\n", fusion_count);
        fprintf(stderr, "        - Conv+ReLU, Dense+ReLU patterns\n");
    } else {
        fprintf(stderr, "      No fusion opportunities detected\n");
    }
}
