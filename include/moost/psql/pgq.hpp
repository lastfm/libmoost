/* vim:set ts=3 sw=3 sts=3 et: */
/**
 * Copyright © 2008-2013 Last.fm Limited
 *
 * This file is part of libmoost.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef MOOST_PSQL_PGQ_CONSUMER_HPP__
#define MOOST_PSQL_PGQ_CONSUMER_HPP__

#include <string>
#include <sstream>
#include <cstring>

#include <pqxx/pqxx>

namespace moost {
namespace psql {

/// This class represents a consumer for a queue in pgq
class pgq_consumer
{
public:
    /**
     * \param dbconn the postgres db connection specification
     * \param queue_name the name of the queue in pgq
     * \param consumer_name the name under which this pgq_consumer object shall register
     */
    pgq_consumer(std::string const &dbconn,
                 std::string const &queue_name,
                 std::string const &consumer_name)
        : m_dbconn(dbconn)
        , m_queue_name(queue_name)
        , m_consumer_name(consumer_name)
    {
    }

    /**
     * \brief Register as a consumer with pgq.
     *
     * This method connects to the database and registers the pgq consumer. If it returns without
     * an exception, the consumer is registered.
     *
     * \return a boolean that indicates whether the consumer was registered by this call. False
     * indicates that the consumer had been registered already.
     */
    bool register_consumer() const
    {
        pqxx::connection conn(m_dbconn);
        std::stringstream query;
        query << "select pgq.register_consumer("
              << conn.quote(m_queue_name) << ','
              << conn.quote(m_consumer_name) << ");";

        pqxx::work transaction(conn, "RegisterPgqConsumer");
        pqxx::result res = transaction.exec(query);
        // If no exception has been thrown, the consumer is registered now.

        /// True if consumer was registered with this query. False if it had been registered already.
        bool const return_value = res.size() == 1 && res[0][0].as(0);

        transaction.commit();

        return return_value;
    }

    /**
     * \brief Unregister the consumer
     *
     * This method connects to the database and unregisters the pgq consumer. If it returns without
     * an exception, the consumer is not registered.
     *
     * \return a boolean that indicates whether the consumer was unregistered by this call. False
     * indicates that the consumer had not been registered at all.
     */

    bool unregister_consumer() const
    {
        pqxx::connection conn(m_dbconn);

        /// True if consumer was registered with this query. False if it had been registered already.
        bool return_value = false;

        try
        {
            std::stringstream query;
            query << "select pgq.unregister_consumer("
                  << conn.quote(m_queue_name) << ','
                  << conn.quote(m_consumer_name) << ");";

            pqxx::work transaction(conn, "UnregisterPgqConsumer");
            pqxx::result res = transaction.exec(query);

            return_value = res.size() == 1 && res[0][0].as(0);

            transaction.commit();
        }
        catch (pqxx::sql_error const & e)
        {
            if (!std::strstr(e.what(), "consumer not registered on queue"))
            {
                throw;
            }
        }

        return return_value;
    }

    /**
     * \brief Check whether consumer is currently registered.
     *
     * This method connects to the database to check whether the consumer is registered with pgq.
     *
     * \return Boolean that indicates whether the consumer is currently registered
     */

    bool is_registered() const
    {
        pqxx::connection conn(m_dbconn);
        std::stringstream query;
        query << "select 1 as ok from pgq.get_consumer_info() where queue_name="
              << conn.quote(m_queue_name) << " and consumer_name="
              << conn.quote(m_consumer_name) << ';';

        pqxx::work transaction(conn, "CheckPgqConsumer");
        pqxx::result res = transaction.exec(query);

        return res.size() == 1 && res[0][0].as(0);
    }

    /**
     * \brief Poll the pgq queue and call a functor with the pqxx::result object.
     *
     * This method connects to the database and polls the pgq queue. As long as it
     * receives non-empty results, it calls a functor with a result set.
     *
     * \param functor The functor to be called for each pqxx::result object.
     */

    template<class FunctorType>
    void poll(char const * columns, FunctorType const & functor) const
    {
        pqxx::connection conn(m_dbconn);

        for(;; )
        {
            std::stringstream query;
            query << "select next_batch from pgq.next_batch("
                  << conn.quote(m_queue_name) << ','
                  << conn.quote(m_consumer_name) << ");";

            pqxx::work transaction1(conn, "PollPgq1");
            pqxx::result res = transaction1.exec(query);

            if (res.empty() || res.front()[0].is_null())
            {
                transaction1.commit();
                return;
            }

            long next_batch;
            res.front()[0].to(next_batch);
            transaction1.commit();

            query.str(std::string());
            query << "select " << columns << " from pgq.get_batch_events(" << next_batch << ");";

            pqxx::work transaction2(conn, "PollPgq2");

            functor(transaction2.exec(query));

            query.str(std::string());
            query << "select pgq.finish_batch(" << next_batch << ");";
            transaction2.exec(query);
            transaction2.commit();
        }
    }

    /**
     * \return The db connection string.
     */

    std::string const & dbconn() const
    {
        return m_dbconn;
    }

    /**
     * \return The name of the pgq queue.
     */

    std::string const & queue_name() const
    {
        return m_queue_name;
    }

    /**
     * \return The name of this pgq consumer.
     */

    std::string const & consumer_name() const
    {
        return m_consumer_name;
    }

private:
    std::string const m_dbconn;
    std::string const m_queue_name;
    std::string const m_consumer_name;
};

} // namespace
} // namespace

#endif // ifndef MOOST_PSQL_PGQ_CONSUMER_HPP__
