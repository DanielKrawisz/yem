#ifndef YEM_BRAIN_HPP
#define YEM_BRAIN_HPP

#include <abstractions/association.hpp>
#include <abstractions/abstractions.hpp>
#include <abstractions/data.hpp>

namespace yem {
    
    using namespace abstractions;
    
    using N = abstractions::N;
    template <typename key, typename value> using map = abstractions::abstract_map<key, value>;
    template <typename X> using set = abstractions::abstract_set<X>;
    template <typename X> using list = abstractions::list<X>;
    
    // Generate a list of fractions up to 1 / Natural;
    template <typename rational>
    struct reciprocal {
        N Natural;
        
        // List of inverses of numbers going back to 1, starting from Natural. 
        list<rational> Reciprocals;
        
        // Total of all fractions from 1 to 1 / n;
        rational Total;
        
        reciprocal() : Natural{0}, Reciprocals{}, Total{0} {} 
        
        reciprocal next() {
            N n = Natural + 1;
            rational r = rational{1} / n;
            list<rational> R = Reciprocals + r;
            return reciprocal{n, R, Total + r};
        }
    };
    
    template <typename rational>
    const reciprocal<rational> reciprocals_to(N n) {
        static list<reciprocal<rational>> Precomputed;
        if (n == 0) return {};
        N max = Precomputed.size();
        if (Precomputed > n) return Precomputed[max - n];
        for (int i = max + 1; i <= n; i ++) Precomputed = Precomputed + Precomputed.next();
        return Precomputed.first();
    }
    
    template <typename user, typename resource, typename rational>
    struct brain {
        set<user> Users;
        
        user Server;
        
        map<resource, list<user>> Upvotes;
        
        rational UpvotePrice;
        
        rational ViewPrice;
        
        rational ServerShare;
    
        using payout = map<user, rational>;
        
        struct outcome {
            brain<user, resource, rational>* Brain;
            payout Payout;
            
            bool valid() const {
                return Brain != nullptr;
            }
            
            outcome() : Brain{nullptr}, Payout{} {}
            outcome(brain b, payout p) : Brain{b}, payout{p} {}
        };
        
        outcome upvote(user u, rational payment, resource r) {
            if (!Users.contains(u) || !Upvotes.contains(r) || payment < UpvotePrice || Upvotes[r].contains(u)) return outcome {};
            
            return outcome{
                // add an upvote for the given user.
                brain{Users, Server, Upvotes.replace(r, Upvotes[r] + u), UpvotePrice, ViewPrice},
                
                // the server takes the payment for the upvote. 
                payout{{{Server, payment}}}};
        }
        
        outcome view(user u, rational payment, resource r) {
            if (!Users.contains(u) || !Upvotes.contains(r) || payment < ViewPrice) return outcome {};
            
            list<user> upvoters = Upvotes[r];
            
            const reciprocal<rational> fractions = reciprocals_to(upvoters.size());
            
            rational total = fractions.Total + ServerShare;
            
            struct user_payouts {
                rational Total;
                
                map<user, rational> operator()(user u, rational r) {
                    return map<user, rational>{{{u, r / Total}}};
                }
            };
            
            return outcome{
                brain{Users, Server, Upvotes, UpvotePrice, ViewPrice}, 
                
                data::map::insert(data::list::inner(user_payouts{total}, upvoters, fractions, data::map::insert), Server, ServerShare / total)};
        }

    };
    
}

#endif
